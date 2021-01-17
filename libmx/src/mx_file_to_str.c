#include "../inc/libmx.h"

char *mx_file_to_str(const char *file) {
	if(!file) return NULL;
	int _file = open(file, O_RDONLY);
	int _file_for_count = open(file, O_RDONLY);
	if(_file < 0){close(_file); return NULL;}
	int filelen = 0;
	char buff;
    while(read(_file_for_count, &buff, 1)) filelen++;
	if(filelen == 0) return NULL;
	char *str = mx_strnew(filelen);
	read(_file, str, filelen);
	close(_file);
	return str;
}
