#include "../inc/libmx.h"

void *mx_memmem(const void *big, size_t big_len, const void *little, size_t little_len){
    if(!little) return 0;
    const char *_big = big;
    const char *_little = little;
    char *ptr = mx_memchr(_big, _little[0], big_len);
    while(*ptr){ if(mx_memcmp(ptr, _little, little_len - 1) == 0) return ptr; ptr++; }
    return 0;
}
