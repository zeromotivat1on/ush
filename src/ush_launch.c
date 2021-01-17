#include "../inc/ush.h"

int ush_launch(char **args){
	int status;
	pid_t pid = fork(), wpid;
	if(pid < 0){
		perror("ush");
	}
	else if(pid == 0){
		execvp(args[0], args);
		exit(EXIT_FAILURE);
	}
	else{
		do{
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	
	return 1;
}

