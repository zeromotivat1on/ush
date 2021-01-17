#include "../inc/libmx.h"

int mx_get_substr_index(const char *str, const char *sub){
    if(!str || !sub) return -2;
    int str_len = mx_strlen(str), sub_len = mx_strlen(sub);
    int found = 0;
    int i, j;
    for (i = 0; i <= str_len - sub_len; ++i) {
        found = 1;
        for (j = 0; j < sub_len; ++j)
            if (str[i + j] != sub[j]) {found = 0; break;}
        if (found) return i;
    }
    return -1;
}
