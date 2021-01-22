#include "ush.h"

char *get_variable_dollar(char *s, int *var_len) {
    char *variable = NULL;
    int i = 0;

    if (!s)
        return NULL;
    if (s[0] == '{') {
        i = mx_get_char_index_quote(s, "}", NULL);
        if (i < 0)
            return NULL;
        variable = mx_strndup(&s[1], i - 1);
        *var_len = mx_strlen(variable) + 2;
    }
    else {
        while (s[i] && !mx_isspace(s[i]) && (isalpha(s[i])
               || isdigit(s[i]) || s[i] == '?' || s[i] =='_'))
            i++;
        if (i != 0)
            variable = mx_strndup(s, i);
        *var_len = i;
    }
    return variable;
}

char *get_value(char *var, Export *variables) {
    for (Export *q = variables; q; q = q->next) {
        if (mx_strcmp(var, q->name) == 0) {
            return q->value;
        }
    }
    return NULL;
}

char *expantion_dillar(char *s, Export *variables, int pos) {
    char *value;
    int v_len = 0;
    char *result = NULL;
    char *var;

    if (!s)
        return NULL;
    result = mx_strndup(s, pos);
    if ((var = get_variable_dollar(&s[pos + 1], &v_len))) {
        if ((value = get_value(var, variables)))
            result = mx_strjoin_free(result, value);
        mx_strdel(&var);
    }
    else{
        result = mx_strjoin_free(result, "");
    }
    if (result && s[pos + v_len + 1])
        result = mx_strjoin_free(result, &s[pos + v_len + 1]);
    mx_strdel(&s);
    return result;
}

char *exp_inside_dblq(char *s, Export *variables, int *i, int *k) {
    char *result = NULL;
    int j = 0;
    int pos = 0;
    char *tmp = NULL;

    (*i) += *k + 1;
    result = mx_strndup(s, *i);
    j = mx_get_char_index_quote(&s[*i], "\"", "`$");
    tmp = mx_strndup(&s[*i], j);

    while (tmp && (pos = mx_get_char_index_quote(tmp, "$", "`$")) >= 0)
        tmp = expantion_dillar(tmp, variables, pos);
        
    result = mx_strjoin_free(result, tmp);
    result = mx_strjoin_free(result, &s[*i + j]);
    (*i) += mx_strlen(tmp) + 1;
    mx_strdel(&tmp);
    mx_strdel(&s);
    return result;
}

char *mx_substr_dollar(char *s, Export *variables) {
    int pos = 0;
    int k = 0;
    char *result = s;
    int i = 0;

    if (!s || !*s || !variables || mx_strcmp(s, "$") == 0)
        return s;
    while (s[i]) {
        if ((k = mx_get_char_index_quote(&result[i], "\"", "\'`$")) >= 0)
            result = exp_inside_dblq(result, variables, &i, &k);
        else
            break;
    }
    while (result && (pos = mx_get_char_index_quote(result, "$", "\"\'`$")) >= 0)
        result = expantion_dillar(result, variables, pos);
    if (!result) {
        mx_printerr("ush: bad substitution\n");
        return NULL;
    }
    return result;
}

char *get_res(char *var, Export *variables) {
    char *res = NULL;
    struct passwd *pw = getpwuid(getuid());

    for (Export *q = variables; q; q = q->next)
        if (mx_strcmp(var, q->name) == 0)
            res = q->value;
    if (!res) {
        if (mx_strcmp(var, "HOME") == 0) {
            return strdup(pw->pw_dir);
        }
        else
            return NULL;
    }
    return mx_strdup(res);
}

char *get_prefix(char *s, int *sleshpos) {
    char *prefix = NULL;
    int sp = -1;

    if (s[1]) {
        sp = mx_get_char_index_quote(&s[1], "/", MX_QUOTE);
        if (sp > 0)
            prefix = mx_strndup(&s[1], sp);
        else
            prefix = mx_strdup(&s[1]);
    }

    *sleshpos = sp;
    return prefix;
}

char *expantion_tilde(char *s, Export *var) {
    char *res = NULL;
    int sleshpos;
    char *prefix = get_prefix(s, &sleshpos);

    if (prefix == NULL)
        res = get_res("HOME", var);
    else if (prefix[0] == '/' && sleshpos == 0)
        res = get_res("HOME", var);
    else if (prefix[0] == '+' && !prefix[1])
        res = get_res("PWD", var);
    else if (mx_strcmp(prefix, "-") == 0)
        res = get_res("OLDPWD", var);
    else
        res = mx_add_login(get_res("HOME", var), prefix);
    if (res && sleshpos >= 0)
        res = mx_strjoin_free(res, &s[sleshpos + 1]);
    mx_strdel(&prefix);
    return res;
}

char *mx_subst_tilde(char *s, Export *variables) {
    char *res = NULL;

    if (!s || !*s)
        return s;
    if (s[0] == '~') {
        res = expantion_tilde(s, variables);
        if (res) {
            mx_strdel(&s);
            return res;
        }
    }
    return s;
}

char *replace_path(char *str) {
    char *res = NULL;
    int len = 0;

    for(int i = mx_strlen(str) - 1; i > 0; i--) {
        if (str[i] == '/')
            break;
        len++;
    }

    res = strndup(str, mx_strlen(str) - len);
    return res;
}

char *mx_add_login(char *home, char *prefix) {
    struct passwd *pw = getpwuid(getuid());
    int i = mx_get_char_index_reverse(home, '/');
    struct stat buff;
    char *path = NULL;

    if (!home || !*home) {
        path = replace_path(pw->pw_dir);
    }
    else
        path = mx_strndup(home, i + 1);

    path = mx_strjoin_free(path, prefix);
    if (lstat(path, &buff) != 0) {
        mx_strdel(&path);
        return NULL;
    }
    
    mx_strdel(&home);
    return path;
}

char *get_sub_str(char *s, int *lenght) {
    int i = 0;
    char *sub_str = NULL;

    if (s[0] == '$' && s[1] == '(') {
        i = mx_get_char_index_quote(s, ")", "\'");
        if (i < 0)
            return NULL;

        sub_str = mx_strndup(&s[2], i - 2);
        *lenght = i;
    }
    else if (s[0] == '`') {
        i = mx_get_char_index_quote(&s[1], "`", "\'");
        if (i < 0)
            return NULL;

        sub_str = mx_strndup(&s[1], i);
        *lenght = i + 1;
    }
    return sub_str;
}

char *expantion_command(char *s, int pos, Prompt *shell) {
    char *sub_str = NULL;
    char *res = mx_strndup(s, pos);
    int len = 0;

    if (s[pos + len + 1])
        res = mx_strjoin_free(res, &s[pos + len + 1]);

    if ((sub_str = get_sub_str(&s[pos], &len))) {
        sub_str = mx_run_sub_shell(sub_str, shell);
        res = mx_strjoin_free(res, sub_str);
        if(sub_str)
            mx_strdel(&sub_str);
    }

    mx_strdel(&s);
    return res;
}

char *mx_sub_str_command(char *s, Prompt *shell) {
    int pos = 0;
    char *result = s;

    if (!s || !*s)
        return s;

    while (result && (pos = mx_get_char_index_quote(result, "`$", "\'")) >= 0) {
        if (!(result = expantion_command(result, pos, shell))) {
            mx_printerr("u$h: command sub_str doesn't work.\n");
            return NULL;
        }
    }

    return result;
}
