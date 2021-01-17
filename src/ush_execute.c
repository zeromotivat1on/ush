#include "../inc/ush.h"

int ush_execute(char **args){
	if(!args){
		return ush_launch(args);
	}

	/*
	 * pwd :
	 *	-L - use pwd from environment, even if it contains symbolic links
	 *	-P - avoid all symbolic links
	 */

	char *builtin_cmd[9] = {
		"export", // 0
		"unset",  // 1
		"exit",   // 2
		"fg",	  // 3
		"env",	  // 4
		"cd",	  // 5
		"pwd",	  // 6
		"which",  // 7
		"echo",	  // 8
	};

	int switch_arg = 0;
	bool cmd_found = false;
	for(int i = 0; i < 9; ++i){
		if(strcmp(args[0], builtin_cmd[i]) == 0){
			switch_arg = i;
			cmd_found = true;
			break;
		}
	}

	if(!cmd_found){
		mx_printerr("ush: command not found: ");
		mx_printerr(args[0]);
		mx_printerr("\n");
		return 1;
	}

	switch(switch_arg){
		case 2: ush_exit(); break;
		case 5: ush_cd(args); break;
		case 6: ush_pwd(args); break;
		case 8: ush_echo(args); break;
	}

	return ush_launch(args);
}

