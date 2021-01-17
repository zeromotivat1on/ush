#include "../inc/ush.h"

char *ush_read_line(){
	
	int bufsize = 128;
	char *buffer = mx_strnew(bufsize);

	int c, pos = 0;

	while (1) {
		c = getchar();

		if (c == EOF || c == '\n'){
			buffer[pos] = '\0';
			return buffer;
		} 
		else{
			buffer[pos] = c;
		}
		pos++;
	}
}

