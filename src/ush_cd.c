#include "../inc/ush.h"

int ush_cd(char **args){
	if(args[1] == NULL){
		mx_printerr("ush: cd: expected argument\n");
	}
	else{
		if(args[2] != NULL){
			mx_printerr("ush: ");
			mx_printerr("cd: ");
			mx_printerr("too many arguments\n");
			return 1;
		}
		if(chdir(args[1]) != 0){
			mx_printerr("ush: ");
			mx_printerr("cd: ");
			mx_printerr(args[1]);
			mx_printerr(": No such file or directory\n");
		}
	}
	
	return 1;
}

