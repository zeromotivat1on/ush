#include "../inc/ush.h"

void ush_loop(){
	char *line;
	char **args;
	int status;

	do{
		printf("u$h> ");
		line = ush_read_line();
		args = ush_parse_line(line);
		status = ush_execute(args);

		free(line);
		free(args);

	} while (status);
}


