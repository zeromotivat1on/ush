#include "../inc/ush.h"

int ush_echo(char **args){
	int quotes_amnt = 0;
	for(int i = 0; args[i]; ++i){
		for(int j = 0; j < mx_strlen(args[i]); ++j){
			if(args[i][j] == '"'){
				quotes_amnt++;
			}
		}
	}

	if(quotes_amnt % 2 != 0){
		mx_printerr("Odd number of quotes.\n");
	}

	return 1;
}

