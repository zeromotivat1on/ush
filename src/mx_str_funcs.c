#include "ush.h"

char *get_end_usual_quote_func(char *s, const char *delim, char *end) {
    while (*s && !(mx_isdelim(*s, (char *)delim))) {
        if (*s == '\\') {
            s += 2;
        }
        else if (*s == '\'') {
            s += mx_get_char_index(s + 1, '\'') + 2;
        }
        else if (mx_isdelim(*s, "\"")) {
            s += mx_get_char_index_quote(s + 1, "\"", "`$") + 2;
        }
        else if (mx_isdelim(*s, "`")) {
            s += mx_get_char_index_quote(s + 1, "`", "\"\'$(") + 2;
        }
        else if (mx_strncmp(s, "$(", 2) == 0) {
            s += mx_get_char_index_quote(s + 2, ")", MX_QUOTE) + 3;
        }
        else if (mx_strncmp(s, "() {", 4) == 0) {
            s += mx_get_char_index_quote(s + 4, "}", MX_QUOTE) + 5;
        }
        else {
            s++;
        }
    }
    end = s;

    return end;
}

char *str_tok_tmp (char *s, const char *delim, char **save_ptr) {
    char *end = NULL;

    if (s == NULL) {
        s = *save_ptr;
    }

    s += strspn(s, delim);
    
    if (*s == '\0') {
        *save_ptr = s;
        return NULL;
    }
    if (!(end = get_end_usual_quote_func(s, delim, end))) {
        return NULL;
    }
    if (*end == '\0') {
        *save_ptr = end;
        return s;
    }

    *end = '\0';
    *save_ptr = end + 1;

    return s;
}

char *mx_strtok (char *s, const char *delim) {
    static char *olds;
    return str_tok_tmp (s, delim, &olds);
}

int mx_strlen_arr(char **s) {
    if (!s) return 0;
    int index = 0;
    for (;s[index];) index++;
    return index;
}

char *mx_strjoin_free(char *s1, char const *s2) {
    char *p = NULL;

    if (!s1 && !s2) {
        return NULL;
    }
    else if (!s1) {
        p = mx_strdup(s2);
    }
    else if (!s2) {
        p = mx_strdup(s1);
    }
    else {
        p = mx_strnew(mx_strlen(s1) + mx_strlen(s2));
        if (!p) {
            return NULL;
        }
        
        p = mx_strcpy(p, s1);
        p = mx_strcat(p, s2);
    }

    mx_strdel(&s1);
    return p;
}



char **mx_strdup_arr(char **strarr) {
    int len;
    int i = 0;

    if (!strarr) {
        return NULL;
    }
    len = mx_strlen_arr(strarr);

    char **res = (char **)malloc((len + 1) * sizeof(char *));
    for (; i < len; i++) {
        res[i] = mx_strdup(strarr[i]);
    }
    res[i] = NULL;
    
    return res;
}

char *mx_strdup_from(char *str, int index) {
    for (int i = 0; i <= index; i++) {
        str++;
    }
    
    return strdup(str);
}

void mx_get_char_auditor(char *str, int *ii_arr, char *q_str) {
    int i = *ii_arr;
    char temp;

    if ((mx_isdelim(str[i], q_str) && str[i] == '`') || (mx_isdelim(str[i], q_str) && str[i] == '\"')) {
        temp = str[i];
        i++;
        while (str[i] && str[i] != temp) {
            (str[i] == '\\') ? (i += 2) : (i++);
        }
    }
    else if (mx_isdelim(str[i], q_str) && str[i] == '\'') {
        i++;
        while (str[i] && str[i] != '\'') {
            i++;
        }
    }

    *ii_arr = i;
}

int mx_get_char_index_quote(char *str, char *c_str, char *q_str) {
    if (!str || !c_str) {
        return -2;
    }
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\\') {
            i++;
        }
        else if (mx_isdelim(str[i], q_str) && !mx_strncmp(&str[i], "$(", 2)) {
            while (str[i] && str[i] != ')') {
                (str[i] == '\\') ? (i += 2) : (i++);
            }
        }
        else if (mx_isdelim(str[i], q_str) && !mx_strncmp(&str[i], "() {", 4)) {
            while (str[i] && str[i] != '}') {
                (str[i] == '\\') ? (i += 2) : (i++);
            }    
        }
        else if (mx_isdelim(str[i], q_str) && mx_isdelim(str[i], "`\'\"")) {
            mx_get_char_auditor(str, &i, q_str);
        }
        else {
            for (int j = 0; j < mx_strlen(c_str); j++) {
                if (str[i] == c_str[j]) {
                    return i;
                }
            }
        }
    }

    return -1;
}

void mx_strtrim_quote_auditor(char *str, char *temp, int *ii_arr, int *jj_arr) {
    int i = *ii_arr;
    int j = *jj_arr;

    if (str[i] == '\"') {
        i++;
        for (; str[i] && str[i] != '\"'; i++, j++) {
            if (str[i] == '\\' && mx_isdelim(str[i + 1], MX_DBLQ_EXCEPTIONS)) {
                i++;
            }
            temp[j] = str[i];
        }
        j--;
    }
    else if (str[i] == '\'') {
        i++;
        for (; str[i] && str[i] != '\''; i++, j++) {
            temp[j] = str[i];
        }
        j--;
    }

    *ii_arr = i;
    *jj_arr = j;
}

void mx_strtrim_quote(char **str) {
    char *temp = NULL;
    int j = 0;
    int i = 0;

    for (int k = 0; str[k]; k++) {
        char *s = str[k];
        temp = mx_strnew(mx_strlen(s));
        for (i = 0, j = 0; s[i]; i++, j++) {
            if (s[i] && s[i] == '\\') {
                i++;
                temp[j] = s[i];
            }
            else if (s[i] && (s[i] == '\"' || s[i] == '\'')) {
                mx_strtrim_quote_auditor(s, temp, &i, &j);
            }
            else {
                temp[j] = s[i];
            }
        }
        mx_strdel(&str[k]);
        str[k] = temp;
    }
}

void mx_print_args_in_line(char **res_arr, const char *delim_str) {
    if (!delim_str || !res_arr) {
        return;
    }

    if (res_arr[0] == NULL) {
        mx_printstr("NULL\n");
    }

    for (int i = 0; res_arr[i] != NULL; i++) {
        mx_printstr(res_arr[i]);
        if (res_arr[i + 1] != NULL) {
            mx_printstr(delim_str);
        }
    }
}
