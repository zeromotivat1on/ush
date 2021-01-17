#include "../inc/libmx.h"

int mx_strncmp(const char *s1, const char *s2, int n){
    while((*s1 == *s2) && s1 && n){++s1; ++s2; --n;}
    if(n == 0) return 0;
    else return (*s1 - *s2);
}
