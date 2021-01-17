#include "../inc/ush.h"

#define USH_TOK_DELIM " \t\n\r\a"

char **ush_parse_line(char *line){
	if(mx_strtrim(line) == NULL || mx_strcmp(line, "") == 0){
		return NULL;
	}

	int bufsize = 128, pos = 0;
	char **tokens = malloc(bufsize * sizeof(char *));
	char *token;

	token = strtok(line, USH_TOK_DELIM);
	while(token != NULL){
		tokens[pos] = token;
		//printf("tokens[%d]:%s\n",pos,tokens[pos]);
		pos++;

		if(pos >= bufsize){
			bufsize += 64;
			tokens = realloc(tokens, bufsize * sizeof(char *));
			if(!tokens){
				fprintf(stderr, "ush: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, USH_TOK_DELIM);
	}

	tokens[pos] = NULL;

	return tokens;
}


