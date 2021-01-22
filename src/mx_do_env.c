#include "ush.h"

char *get_parameter(char **args, int *indx, int j_index, int *option_n) {
    char *param = NULL;

    if(args[*indx][j_index + 1] != '\0') {
        param = mx_strdup_from(args[*indx], j_index);
    } else if (args[*indx + 1]) {
        param = mx_strdup(args[*indx + 1]);
        (*option_n)++;
        (*indx)++;
    }
    return param;
}

int add_parameter(char *param, Export **env_params, char option) {
    char *tmp_option = malloc(2*sizeof(char));;

    if (param) {
        if (mx_strchr(param, '=') && option == 'u') {
            mx_printerr("env: unsetenv ");
            mx_printerr(param);
            mx_printerr(": Invalid argument\n");
            return -1;
        }
        tmp_option[0] = option;
        tmp_option[1] = '\0';
        mx_push_export(env_params, tmp_option, param);
        free(param);
        free(tmp_option);
        return 0;
    } else {
        mx_print_env_error(option, "env: option requires an argument -- ");
        return -1;
    }
}

int mx_add_option(char **args, int *i, int *option_n, BuiltIn *env) {
    int flag = 1;
    char *parametr = NULL;
    int exit_index = 0;

    for (int j = 1; j <= mx_strlen(args[*i]) && flag != 0; j++) {
        if (args[*i][j] == 'P' || args[*i][j] == 'u' ) {
            parametr = get_parameter(args, i, j, option_n);
            exit_index = add_parameter(parametr, &env->env_params, args[*i][j]);
            flag = 0;
        } else if (args[*i][j] == '-' || args[*i][j] == 'i' || mx_strlen(args[*i]) == 1){
            env->env_options.i = 1;
        } else if (j != mx_strlen(args[*i])){
            mx_env_err(&flag, &exit_index, args[*i][j]);
        }
    }
    return exit_index;
}

int mx_count_env_options(char **args, BuiltIn *env) {
    int n_options = 0;

    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] != '-') break;
        if (strcmp(args[i], "--") == 0) {
            n_options++;
            break;
        }
        int valid_arg = mx_add_option(args, &i, &n_options, env);
        if (valid_arg < 0)
            return -1;
        n_options++;
    }
    return n_options;
}

void mx_env_err(int *flag, int *exit_code, char option) {
    mx_print_env_error(option, "env: illegal option -- ");
    (*exit_code) = -1;
    (*flag)--;
}

void mx_print_env_error(char option, char *error) {
    mx_printerr(error);
    write(2, &option, 1);
    mx_printerr("\nusage: env [-iv] [-P utilpath] [-u name]\n");
    mx_printerr("           [name=value ...] [utility [argument ...]]\n");
}

static int count_variables(char **args, int option_num) {
    int variable_num = 0;
    for (int i = option_num + 1; args[i] != NULL; i++) {
        if (mx_get_char_index(args[i],'=') < 0) break;

        if (mx_get_char_index(args[i],'=') == 0) {
            printf("env: setenv %s: Invalid argument\n", args[i]);
            return -1;
        }
        variable_num++;
    }
    return variable_num;
}

static int env_option_num(char **args, int n_all) {
    int n_args = 0;
    for (int i = n_all + 1; args[i] != NULL; i++) n_args++;
    return n_args;
}

static void env_print(Export *env_list) {
    Export *head = env_list;
    while (head != NULL) {
        printf("%s=%s\n", head->name, head->value);
        head = head->next;
    }
}

static BuiltIn *init_env (Process *p) {
    BuiltIn *env = (BuiltIn *)malloc(sizeof(BuiltIn));

    env->env_params = NULL;
    env->env_list = NULL;
    env->env_options.P = 0;
    env->env_options.i = 0;
    env->env_options.u = 0;
    env->n_options = mx_count_env_options(p->argv, env);
    env->n_variables = count_variables(p->argv, env->n_options);
    env->n_args = env_option_num(p->argv, env->n_variables + env->n_options);
    env->path = NULL;
    return env;
}

int mx_env(Prompt *shell, Process *p) {
    BuiltIn *env = init_env(p);
    int exit_point = 0;

    mx_set_data(env, p->argv);
    if (env->n_options < 0 || env->n_variables < 0)  exit_point = 1;
    else if (env->n_args == 0) env_print(env->env_list); 
    else mx_launch_command(p, env, &exit_point);

    if (env->path) free (env->path);

    shell->exit_flag = 0;
    mx_clear_export(env->env_list);
    mx_clear_export(env->env_params);
    free(env);
    return exit_point;
}

void env_get_data(int i, char **args, BuiltIn *env) {
    int idx = mx_get_char_index(args[i],'=');
    char *name = strndup(args[i],idx);
    char *value = mx_strdup_from(args[i],idx);

    mx_push_export(&env->env_list, name, value);
    free(name);
    free(value);
}

void env_delete_name(Export **list, char *arg) {
    Export *head = *list;

    if (head != NULL && strcmp(head->name, arg) == 0) {
        *list = (*list)->next;
        return;
    }
    while (head != NULL) {
        if (head->next != NULL) {
            if (strcmp(head->next->name, arg) == 0) {
                free(head->next->name);
                free(head->next->value);
                free(head->next);
                head->next = head->next->next;
                break;
            }
        }
        head = head->next;
    }
}

void env_get_params (Export *env_params, Export *env_list, BuiltIn *env) {
    Export *head = env_params;

    while (head != NULL) {
        if (strcmp(head->name, "u") == 0) {
            env_delete_name(&env_list, head->value);
        }
        if (strcmp(head->name, "P") == 0) {
            if (env->path != NULL) free(env->path);
            env->path = strdup(head->value);
        }
        head = head->next;
    }
}

void mx_set_data(BuiltIn *env, char *args[]) {
    extern char** environ;
    for (int i = 0; environ[i] != NULL; i++) {
        if (!env->env_options.i) env_get_data(i, environ, env);
    }
    env_get_params(env->env_params, env->env_list, env);
    for (int i = env->n_options + 1; i <= env->n_options + env->n_variables; i++) {
        env_get_data(i, args, env);
    }
}

char **env_get_args(Process *process, int start) {
    char **args_arr = (char **)malloc(sizeof(char *) * 256);
    int tmp_start = start;

    while (process->argv[tmp_start]) {
        args_arr[tmp_start - start] = strdup(process->argv[tmp_start]);
        tmp_start++;
    }
    mx_del_strarr(&process->argv);
    args_arr[tmp_start - start] = NULL;
    return args_arr;
}

char **get_env_arr(Export *env_list) {
    char **env_arr = (char **)malloc(sizeof(char *) * 1024);
    Export *head = env_list;
    int i = 0;
    char *str = NULL;

    while (head != NULL) {
        str = mx_strdup(head->name);
        str = mx_strjoin(str, "=");
        str = mx_strjoin(str, head->value);
        env_arr[i] = strdup(str);
        i++;
        head = head->next;
        free(str);
    }
    env_arr[i] = NULL;
    return env_arr;
}

void mx_launch_command( Process *process, BuiltIn *env, int *exit_code) {
    char **args_arr = env_get_args(process, env->n_options + env->n_variables + 1);
    char **env_arr = get_env_arr(env->env_list);

    if (!env->path) env->path = strdup(getenv("PATH"));
    process->argv = args_arr;
    *exit_code = mx_launch_bin(process, env->path, env_arr);
    mx_del_strarr(&env_arr);
}
