#include "../inc/ush.h"

int ush_launch(char **args){
	pid_t pid = fork();
	if(pid < 0){
		perror("ush");
	}
	else if(pid == 0){
		execvp(args[0], args);
		//exit(EXIT_FAILURE);
	}
	else{
		wait(NULL);	
	}
	
	return 1;
}

