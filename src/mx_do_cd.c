#include "ush.h"

char *replace_sub(char *str, char *sub, char *replace) {
    char *result;
    int index = mx_get_substr_index(str,sub);
    char *buff_1 = strndup(str, index);
    char *buff_2;
    char *tmp = NULL;

    for (int i = 0; i < index + mx_strlen(sub); i++) str++;
    
    buff_2 = strdup(str);
    result = mx_strjoin(buff_1, replace);
    tmp = mx_strjoin(result,buff_2);
    free(result);
    result = strdup(tmp);
    free(tmp);
    free(buff_1);
    free(buff_2);
    return(result);
}

char *chpwd(char **args, int option_num, Prompt *shell) {
    char *current = strdup(shell->pwd);
    char *new = NULL;

    if (!mx_strstr(current, args[option_num + 1])) {
        mx_printerr("ush: cd: string not in pwd: ");
        mx_printerr(args[option_num + 1]);
        mx_printerr("\n");
    }
    else {
        new = replace_sub(current, args[option_num + 1], args[option_num + 2]);
    }
    free(current);
    return new;
}

void cd_fill_options(int option_num, cd_t *cd_options, char **args) {
    for(int i = option_num; i > 0; i --) {
        for (int j = mx_strlen(args[i]); j > 0; j--) {
            if (args[i][j] == 'L' && cd_options->P <= 0) cd_options->L = 1;
            if (args[i][j] == 'P' && cd_options->L <= 0) cd_options->P = 1;
            if (args[i][j] == 's') cd_options->s = 1;
        }
    }
}

int cd_count_args(char **args, int option_num) {
    int args_num = 0;

    for (int i = option_num + 1; args[i]; i++) args_num++;

    if (args_num > 2){
        mx_printerr("ush: cd: too many arguments\n");
        return 0; 
    }
    return args_num;
}

int mx_cd(Prompt *shell, Process *p) {
    cd_t cd_options = {0, 0, 0};
    int exit_point = 1;
    int option_num = mx_count_options(p->argv, "sLP", "cd", " [-s] [-L|-P] [dir]");
    int args_num = cd_count_args(p->argv, option_num);
    char *point = NULL;

    cd_fill_options(option_num, &cd_options, p->argv);

    if (option_num >= 0 && args_num < 3) {
        if (args_num == 0) point = mx_go_home();
        else if (args_num == 2) point = chpwd(p->argv, option_num, shell);
        else point = mx_go_somewhere(p, option_num);
        
        if (point != NULL) mx_change_dir(point, cd_options, shell, &exit_point);
        free(point);
    }
    return exit_point;
}

void manage_env(char *dir, Prompt *shell,  cd_t cd_options, int *exit_code) {
    
    char *link = malloc(1024);
    readlink(dir, link, 1024);

    if (cd_options.P == 1 && strcmp(link, "") != 0) {
        free(dir);
        dir = getcwd(NULL, 1024);
    }

    free(link);
    setenv("OLDPWD", shell->pwd, 1);
    mx_set_variable(shell->variables, "OLDPWD", shell->pwd);
    mx_set_variable(shell->exported, "OLDPWD", shell->pwd);
    setenv("PWD", dir, 1);
    mx_set_variable(shell->variables, "PWD", dir);
    mx_set_variable(shell->exported, "PWD", dir);
    free(shell->pwd);
    shell->pwd = strdup(dir);
    free(dir);
    (*exit_code) = 0;
}

void print_error_cd(char *pointer) {
    mx_printerr("ush: cd: ");
    perror(pointer);
}

int cd_check_path(char *pointer, cd_t cd_options) {
    int flag = 0;
    char *read_link = realpath(pointer, NULL);

    if (cd_options.s) {
        if(read_link && strcmp(pointer, read_link) != 0) {
            mx_printerr("ush: cd: ");
            mx_printerr(pointer);
            mx_printerr(": Not a directory");
            mx_printerr("\n");
            flag++;
        }
    }
    free(read_link);
    return flag;
}

void mx_change_dir(char *pointer, cd_t cd_options, Prompt *shell, int *exit_code){
    char *dir = mx_normalization(pointer, shell->pwd);
    int flag = cd_check_path(pointer, cd_options);

    if (!flag) {
        if (chdir(dir) != 0) {
            print_error_cd(pointer);
            free(dir);
        }
        else manage_env(dir, shell, cd_options, exit_code);
    } else {
        free(dir);
    }
    free(shell->git);
    if (getenv("HOME")) shell->git = mx_get_git_info();
}

char *mx_go_somewhere(Process *p, int option_num) {
    char *pointer = NULL;

    if (strcmp(p->argv[option_num + 1], "-") == 0)
        pointer = mx_go_back();
        else
        pointer = strdup(p->argv[option_num + 1]);
    return pointer;
}

char *mx_go_back(void) {
    char *pointer = NULL;

    if (getenv("OLDPWD")) pointer = strdup(getenv("OLDPWD"));
    else mx_printerr("ush: cd: OLDPWD not set\n");

    return pointer;
}

char *mx_go_home(void) {
    char *pointer = NULL;

    if (getenv("HOME")) pointer = strdup(getenv("HOME"));
    else mx_printerr("ush: cd: HOME not set\n");

    return pointer;
}
