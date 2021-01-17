#include "../inc/libmx.h"

void *mx_memchr(const void *s, int c, size_t n){
    const char *_s = s;
    if(n == 0) return 0;
    for(size_t i = 0; i < n; ++i){
        if(_s[i] == c) return (char*)&_s[i];
    }
    return 0;
}
