#pragma once 
#ifndef USH_H
#define USH_H

#include "libmx.h"

#include <string.h>

#include <sys/wait.h>
#include <sys/ioctl.h>
//#include

// Main loop
void ush_loop();

// Obtain user input
char *ush_read_line();
char **ush_parse_line(char *line);

// Execution
int ush_execute(char **args);
int ush_launch(char **args);

// Commands
int ush_cd(char **args);
int ush_echo(char **args);
int ush_pwd(char **args);
int ush_exit();

// Additional
void mx_printerr(const char *str);

#endif
