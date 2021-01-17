#include "../inc/libmx.h"

void *mx_memcpy(void *restrict dst, const void *restrict src, size_t n){
    unsigned char *_dst = dst;
    const char *_src = src;
    for(size_t i = 0; i < n; ++i)
        _dst[i] = _src[i];
    return _dst;
}
