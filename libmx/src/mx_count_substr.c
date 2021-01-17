#include "../inc/libmx.h"

int mx_count_substr(const char *str, const char *sub) {
    if(!sub || !str) return -1;
    if(mx_strcmp(sub, "") == 0) return 0;
    int str_len = mx_strlen(str), sub_len = mx_strlen(sub);
    int count = 0, found = 0;
    for (int i = 0; i <= str_len - sub_len; ++i) {
        found = 1;
        for (int j = 0; j < sub_len; ++j)
            if (str[i + j] != sub[j]) {found = 0; break;}
        if (found) {count++; i = i + sub_len - 1;}
    }
    return count;
}
