#include "../inc/libmx.h"

int mx_memcmp(const void *s1, const void *s2, size_t n){
    const char *_s1 = s1;
    const char *_s2 = s2;
    if(n == 0) return 0;
    for(size_t i = 0; i < n; ++i)
        if(_s1[i] != _s2[i]) return (_s1[i] - _s2[i]);
    return 0;
}
