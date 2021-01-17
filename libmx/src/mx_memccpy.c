#include "../inc/libmx.h"

void *mx_memccpy(void *restrict dst, const void *restrict src, int c, size_t n){
    unsigned char *_dst = dst;
    const char *_src = src;
    for(size_t i = 0; i < n; ++i){
        _dst[i] = _src[i];
        if(_src[i] == (unsigned char)c) return (void*)&(_dst[i + 1]);
    }
    return NULL;
}
