#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "Socket.h"
#include<signal.h>
#include<sys/stat.h>
#define MAX_LENGTH 65536
#define MAX_ARG 65536
#define MAX_TMP 100
ServerSocket welcome_socket;
Socket connect_socket;
char* str;
char** argv;
char** parse(char* str, char* delim){
	char **argv = malloc(MAX_ARG * sizeof(char *));
	if(argv == NULL)
		exit(133);  //Malloc failed
	char* token;
	int count = 0;
	str[strlen(str) - 1] = '\0';
	token = strtok(str, delim);
	while(token){
		if(count >= MAX_ARG)
			exit(134);  //Too many arguments
		argv[count++] = token;
		token = strtok(NULL, delim);
	}
	argv[count] = NULL;
	free(token);
	return argv;
}
void send(int c, Socket socket){
    int rc = Socket_putc(c, socket);
    if (rc == EOF) {
        printf("Socket_putc EOF or error\n");
        exit(-1);
    }
}
void putmsg(char* msg){
    for(int i = 0; i < strlen(msg); i++){
        int rc = Socket_putc(msg[i], connect_socket);
        if (rc == EOF) {
          printf("Socket_putc EOF or error\n");
            exit(-1);
        }
        // send(msg[i], connect_socket);
    }
}
int fetch(Socket socket){
    int c = Socket_getc(socket);
    if (c == EOF) {
        printf("Socket_getc EOF or error\n");
        Socket_close(socket);
        exit(-1); 
    }
    return c;
}
void service();

int main(int argc, char *argv[]) {
    pid_t spid, term_pid;
    int chld_status;
    if (argc < 2) {
        printf("No port specified\n");
        return (-1);
    }
    welcome_socket = ServerSocket_new(atoi(argv[1]));
    if (welcome_socket < 0) {
        printf("Failed new server socket\n");
        return (-1);
    }

    
    // signal(SIGCHLD, SIG_IGN);
    while (1) {
        connect_socket = ServerSocket_accept(welcome_socket);    
        if (connect_socket < 0) {
             printf("Failed accept on server socket\n");
            exit(-1);
        }
        spid = fork(); 
        if (spid == -1) {
            exit(-1);
        }
        else if (spid == 0) {
            service();
            Socket_close(connect_socket);
            exit(0);
        }
        else 
        {
            // int status = 0;
            Socket_close(connect_socket);
            term_pid = waitpid(-1, &chld_status, WNOHANG);
            // if (term_pid == -1)
            //     status = 137;                   //Waitpid failed
            // else {
            //     if (WIFEXITED(chld_status)) waitpid(term_pid, 0, 0);
            // }
        }
    } 
    Socket_close(welcome_socket);
    return 0;
}
char line_data[MAX_LENGTH + 1];
unsigned char new_line[MAX_LENGTH + 1];
unsigned char tmp_name[MAX_TMP];
unsigned char id_str[MAX_TMP];

void service(){
    pid_t spid, term_pid;
    int chld_status;
    int i, c;
    struct stat path_stat, file_path;
    int id;
    FILE *tmpFP;
    FILE *fp;

    if (connect_socket < 0) {
      printf("Failed accept on server socket\n");
      exit(-1);
    }
    Socket_close(welcome_socket);
    // signal(SIGCHLD, SIG_DFL);
    while (1) {
        i = 0;
        while(i < MAX_LENGTH){
            c = Socket_getc(connect_socket);
            if (c == EOF) 
                break;
            else {
                line_data[i++] = c;
                if(c == '\0') break;
            }
        }
        if(c == EOF) break;
        if (i == MAX_LENGTH) {
            putmsg("Command too long!\n");
            send('\0', connect_socket);
            send(0, connect_socket);
			continue;
        }

        id = (int)getpid();
        sprintf(id_str, "%d", id);
        strcpy(tmp_name, "tmp");
        strcat(tmp_name, id_str);
        spid = fork(); 
        if (spid == -1) {
            exit(135);        //Fork failed
        }
        else if (spid == 0) {
            fp = freopen(tmp_name, "w", stdout);
            argv = parse(line_data, " \t");
            stat(argv[0], &path_stat);
			if(S_ISDIR(path_stat.st_mode))
				exit(136);                  //Is a directory
            if(execvp(argv[0], argv) < 0)
                exit(errno);                //Other errors
            exit(errno);
        }
        else 
        {
            int status = 0;
            term_pid = waitpid(spid, &chld_status, 0);
            if (term_pid == -1)
                status = 137;                   //Waitpid failed
            else 
                if (WIFEXITED(chld_status)) status = WEXITSTATUS(chld_status);

            if ((tmpFP = fopen(tmp_name, "r")) == NULL)
                status = 138;                   //Error opening tmp file
            else{
                while (!feof(tmpFP)) {
                    if (fgets(new_line, sizeof(new_line), tmpFP) == NULL) break;
                    putmsg(new_line);
                }
                fclose(tmpFP);
            }
            printf("%d\n", status);
            send('\0', connect_socket);
            send(status, connect_socket);
        }
    } 
    remove(tmp_name);
    Socket_close(connect_socket);
    // free(tmpFP);
    // free(fp);
    exit(0);
}
