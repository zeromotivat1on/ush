#include "../inc/libmx.h"

char *mx_strstr(const char *haystack, const char *needle){
    if(mx_strlen(needle) == 0) return (char *)haystack;
    char *ptr = mx_strchr(haystack, needle[0]);
    while(ptr != 0){
        if(mx_strncmp(ptr, needle, mx_strlen(needle)) == 0) return (char *)ptr;
        else ptr = mx_strchr(ptr + 1, needle[0]);
    }
    return 0;
}
