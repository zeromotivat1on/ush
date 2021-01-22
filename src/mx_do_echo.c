#include "ush.h"

static int count_options(char **args) {
    int option_num = 0;
    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-') {
            if (!strchr("neE", args[i][1])) break;
        } else break;
        option_num++;
    }
    return option_num;
}

static void fill_options(char **args, echo_t *echo_options, int option_num) {
    for (int i = option_num; i >= 0; i--) {
        for (int j = mx_strlen(args[i]); j >= 1; j--) {
            if (args[i][j] == 'n') {
                echo_options->n = 1;
            } else if (args[i][j] == 'e') {
                if (!echo_options->E) echo_options->e = 1;
            } else if (args[i][j] == 'E') {
                if (!echo_options->e) echo_options->E = 1;
            }
        }
    }
}

static void edit_argv(int option_num, Process *proc) {
    echo_t echo_options = {0, 0, 0};

    fill_options(proc->argv, &echo_options, option_num);
    for(int i = option_num + 1; proc->argv[i] != NULL; i++) {
        if (!echo_options.E && strstr(proc->argv[i], "\\")) mx_escape_seq(proc, i, echo_options);
        printf("%s",proc->argv[i]);
        if (strstr(proc->argv[i],"\\c")) break;
        if (proc->argv[i + 1]) mx_printstr(" ");
    }
    if(!echo_options.n) printf("\n");
}

int mx_echo(Prompt *m_s, Process *proc) {
    m_s->exit_code = 0;
    int exit_code = m_s->exit_code;
    int option_num = count_options(proc->argv);
    edit_argv(option_num, proc);
    return exit_code;
}

static char *get_result(char *buff1, char *buff2,  char *replace) {
    char *tmp = NULL;
    char *result = NULL;

    tmp = strdup(buff1);
    result = mx_strjoin(tmp, replace);
    free(tmp);
    tmp = strdup(result);
    free(result);
    result = mx_strjoin(tmp, buff2);
    free(tmp);
    return result;
}

static char *replace_substr(char *str,  char *sub, char *replace) {
    char *result = strdup(str);
    char *buff1 = NULL, *buff2 = NULL;
    int substr_index = 0;

    while(mx_strstr(result,sub) != NULL) {
        substr_index = mx_get_substr_index(result,sub);
        buff1 = strndup(result, substr_index);
        buff2 = mx_strdup_from(result, substr_index + mx_strlen(sub) - 1);
        free(result);
        result = get_result(buff1, buff2, replace);
        free(buff1);
        free(buff2);
    }
    return result;
}

static void rep_x(int *i, char *str) {
    if (str[*i] == '\\' && str[*i + 1] == 'e') {
        if (str[*i + 2] != '\\') (*i) += 3;
        else (*i) += 2;
    }
}

static char *replace_slash(char *str, echo_t *echo_options) {
    char *result = (char *)malloc(mx_strlen(str) + 1);
    int length = 0;

    for (int i = 0; i <= mx_strlen(str); i++) {
        if (str[i] == '\\' && str[i + 1] == '\\') i++;
        rep_x(&i, str);
        if (str[i] == '\\' && str[i + 1] == 'c') {
            echo_options->n = 1;
            break;
        }
        result[length] = str[i];
        length++;
    }
    result[length] = '\0';
    return result;
}

void mx_escape_seq(Process *proc, int i, echo_t echo_options) {
    char *tmp = replace_slash(proc->argv[i], &echo_options);
    char *sequenses[] = {"\\a","\\b","\\f","\\n","\\r","\\t","\\v",NULL};
    char *escape[] = {"\a","\b","\f","\n","\r","\t","\v",NULL};

    free(proc->argv[i]);
    proc->argv[i] = strdup(tmp);
    free(tmp);
    for (int j = 0; sequenses[j] != NULL; j++) {
        if (strstr(proc->argv[i],sequenses[j])) {
            tmp = replace_substr(proc->argv[i],sequenses[j], escape[j]);
            free(proc->argv[i]);
            proc->argv[i] = strdup(tmp);
            free(tmp);
        }
    }
}
