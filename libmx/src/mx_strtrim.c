#include "../inc/libmx.h"

char *mx_strtrim(const char *str) {
    if (!str) return NULL;
    int size = mx_strlen(str);
    int space_before_str = 0;
    int space_after_str = 0;
    for (int i = 0; mx_isspace(str[i]); ++i) space_before_str++;
    for (int i = mx_strlen(str) - 1; mx_isspace(str[i]); --i) space_after_str++;
    int new_size = size - (space_before_str + space_after_str);
    char *result = mx_strnew(new_size);
    return mx_strncpy(result, &str[space_before_str], new_size);
}
