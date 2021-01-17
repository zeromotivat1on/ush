#include "../inc/libmx.h"

void *mx_memmove(void *dst, const void *src, size_t len){
    unsigned char *_dst = malloc(len);
    _dst = mx_memcpy(_dst, src, len);
    dst = mx_memcpy(dst, _dst, len);
    free(_dst);
    return dst;
}
