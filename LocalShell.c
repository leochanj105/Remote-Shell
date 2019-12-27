#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<signal.h>
#include<sys/stat.h>
#define MAX_ARG 65536
char* str;
char** argv;
size_t len = 0;
int errmsg;
//void stop_handler(int sig){
//	printf("\n");
//	exit(0);
//}

void execute(char* file, char** argv){
	if(execv(file, argv) < 0){
		errmsg = errno;
		fprintf(stderr, "%s\n", strerror(errmsg));
		free(argv);
		exit(1);
	}
}


char** parse(char* str, char* delim){
	char **argv = malloc((MAX_ARG + 1) * sizeof(char *));
	if(argv == NULL){
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	char* token;
	int count = 0;
	str[strlen(str) - 1] = '\0';
	token = strtok(str, delim);
	while(token){
		argv[count++] = token;
		token = strtok(NULL, delim);
	}
	argv[count] = NULL;
	free(token);
	return argv;
}
int main(int argc, char* argv[]){
	signal(SIGINT, SIG_DFL);
	signal(SIGTSTP, SIG_DFL);
	int status;
	struct stat file_path;
	while(1){
		printf("%c ", '%');
		if(getline(&str, &len, stdin) == -1) break;
		signal(SIGINT, SIG_IGN);
		signal(SIGTSTP, SIG_IGN);
		pid_t pid = fork();
		if(pid == -1){
			fprintf(stderr, "Fork failed\n");
			exit(1);
		}
		else if(pid == 0){
			signal(SIGINT, SIG_DFL);	
			signal(SIGTSTP, SIG_DFL);
			argv = parse(str, " \t");
			if(strchr(argv[0], '/') != NULL){
				if(stat(argv[0], &file_path) != 0){
					fprintf(stderr, "File or directory doesn't exist!\n");
					exit(1);
				}
				if(S_ISDIR(file_path.st_mode)){
					fprintf(stderr, "Is a directory\n");
					exit(1);
				}
				execv(argv[0], argv);
			}
			else{
				char *path = getenv("PATH");
				if(path == NULL)
					path = ":/bin:/usr/bin";
				int filelen = strlen(argv[0]) + 1;
				int pathlen = strlen(path);
				char *tmp = malloc((filelen + pathlen) * sizeof(char) ); 
				char *whole = memcpy(tmp + pathlen + 1, argv[0], filelen);
				*--whole = '/';
				char *pos;
				pos = path;
				do{
					char *start;
					path = pos;
					pos = strchr(path, ':');
					if(!pos)
						pos = strchr(path, '\0');
					if(pos == path)
						start = tmp + 1;
					else
						start = memcpy(whole - (pos - path), path, pos - path);
					//printf("%s\n", path);
					//printf("%s\n", start);
					if(stat(start, &file_path) == 0){
						if(!S_ISDIR(file_path.st_mode))
							execute(start, argv);
					}
				}while(*pos++ != '\0');
				fprintf(stderr, "Wrong path or is directory\n");
				exit(1);
			}
			
		}
		waitpid(pid, &status, WUNTRACED);
		signal(SIGINT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
	}
	printf("\n");
	free(str);
	return 0;
}

