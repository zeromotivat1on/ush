#include "../inc/ush.h"

char *ush_read_line(){
	int bufsize = 1024;
	int position = 0;
	char *buffer = malloc(bufsize);
	int c;

	if (!buffer) {
		mx_printerr("ush: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		// Read a character
		c = getchar();

		// If we hit EOF, replace it with a null character and return
		if (c == EOF || c == '\n') {
			buffer[position] = '\0';
			return buffer;
		} 
		else {
			buffer[position] = c;
		}
		position++;

		// If we have exceeded the buffer, reallocate.
		if (position >= bufsize) {
			bufsize += 1024;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

