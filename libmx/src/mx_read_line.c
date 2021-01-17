#include "../inc/libmx.h"

int mx_read_line(char **lineptr, size_t buf_size, char delim, const int fd){
    ssize_t curr_byte;
    size_t tot_bytes = 0;
    char *buffer = *lineptr;
    char c;
    if (buf_size <= 0 || buffer == NULL) return -1;
    while(1){
        curr_byte = read(fd, &c, 1);
        if (curr_byte == -1) return -1;
        else if (curr_byte == 0) {
            if (tot_bytes == 0) return -1;
            else break;
        } else {
            if (c == delim) break;
            if (tot_bytes < buf_size) {
                tot_bytes++;
                *buffer++ = c;
            }
        }
    }
    *buffer = '\0';
    return tot_bytes;
}
