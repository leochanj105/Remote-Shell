#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Socket.h"
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<signal.h>
#include<sys/stat.h>
#define MAX_LENGTH 65536
size_t len = 0;
Socket connect_socket;
// void send(int c, Socket socket){
    
// }
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


int main(int argc, char *argv[]) {
  int i, c, rc;
  int count = 0;
  int errmsg;
  char* line_data;
  

  if (argc < 3) {
    printf("No host and port\n");
    return (-1);
  }

  connect_socket = Socket_new(argv[1], atoi(argv[2]));
  if (connect_socket < 0) {
    printf("Failed to connect to server\n");
    return (-1);
  }

  int status;
  while (1) {
    printf("%c ", '%');
	if(getline(&line_data, &len, stdin) == -1) break;
    count = strlen(line_data); 
    if(count > MAX_LENGTH || line_data[count - 1] != '\n'){
		fprintf(stderr, "Command too long!\n");
		continue;
	}
    for (i = 0; i <= count; i++) {
        int rc = Socket_putc(line_data[i], connect_socket);
        if (rc == EOF) {
            printf("Socket_putc EOF or error\n");
            exit(-1);
        }
    }

    while((c = fetch(connect_socket)) != '\0')
        putchar(c);
    int status = fetch(connect_socket);
    if(status > 132){
        switch(status){
            case 133:
                printf("Malloc failed\n");
                break;
            case 134:
                printf("Too many arguments\n");
                break;
            case 135:
                printf("Fork failed\n");
                break;
            case 136:
                printf("Is a directory\n");
                break;
            case 137:
                printf("Waitpid failed\n");
                break;
            case 138:
                printf("Error opening tmp file\n");
                break;
            case 139:
                printf("Command too long\n");
                break;    
            case 140:
                printf("File or directory doesn't exist!\n");
                break;   
            case 141:
                printf("Wrong path or is directory\n");
                break;   
        }
    }
    else
        if(status != 0)
            printf("%s\n", strerror(status));
        
  } 
  Socket_putc(255, connect_socket);
  Socket_close(connect_socket);
  exit(0);
}
