#include "../inc/libmx.h"

void *mx_realloc(void *ptr, size_t size){
    size_t old_size = malloc_size(ptr);
    if(!size) {free(ptr); return NULL;}
    else if(!ptr){return malloc(size);}
    else if(size <= old_size) return ptr;
    void *_ptr = malloc(size);
    if(_ptr){
        mx_memcpy(_ptr, ptr, old_size);
        free(ptr);
    }
    return _ptr;
}
