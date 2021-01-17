#include "../inc/ush.h"

void ush_loop(){
	// For one line input
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	char *line;
	char **args;
	int status;

	do{
		printf("u$h> ");
		line = ush_read_line();
		if(mx_strlen(line) > w.ws_col - 5){
			mx_printerr("ush: ush supports only one line user input\n");
			free(line);
		}
		else{
			args = ush_parse_line(line);
			status = ush_execute(args);

			free(line);
			free(args);
		}

	} while (status);
}


