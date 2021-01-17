#include "../inc/libmx.h"

char *mx_del_extra_spaces(const char *str) {
    if (!str) return NULL;
    char *mem = mx_strnew(mx_strlen(str));
    char *result = NULL;
    int j = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        if (!(mx_isspace(str[i]))) {
            mem[j] = str[i];
            j++;
        }
        if (!(mx_isspace(str[i])) && mx_isspace(str[i + 1])) {
            mem[j] = ' ';
            j++;
        }
    }
    result = mx_strtrim(mem);
    mx_strdel(&mem);
    return result;
}
