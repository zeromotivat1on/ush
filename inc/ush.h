#pragma once 
#ifndef USH_H
#define USH_H

#include "libmx.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/wait.h>
//#include

void ush_loop();

char *ush_read_line();
char **ush_parse_line(char *line);

int ush_execute(char **args);
int ush_launch(char **args);

int ush_cd(char **args);
int ush_echo(char **args);

void mx_printerr(const char *str);

#endif
