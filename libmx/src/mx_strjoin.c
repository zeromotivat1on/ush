#include "../inc/libmx.h"

char *mx_strjoin(char const *s1, char const *s2){
    if(s1 == NULL && s2 == NULL) return NULL;
    else if(s1 == NULL && s2 != NULL) return mx_strdup(s2);
    else if(s1 != NULL && s2 == NULL) return mx_strdup(s1);
    else{
        char *temp1 = mx_strdup(s1);
        char *temp2 = mx_strdup(s2);
        char *res = mx_strnew(mx_strlen(s1) + mx_strlen(s2));
        res = mx_strcat(temp1, temp2);
        mx_strdel(&temp1);
        mx_strdel(&temp2);
        return res;
    }
}
