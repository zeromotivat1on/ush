#include "ush.h"

void options_fill_pwd(int n_options, pwd_t *pwd_options, char ** args) {
    int l_index = -1;
    int p_index = -1;

    for(int i = n_options; i > 0; i --) {
        for (int j = mx_strlen(args[i]); j > 0; j--) {
            if (args[i][j] == 'L' && p_index < 0) {
                l_index = 1;
            }
            if (args[i][j] == 'P' && l_index < 0) {
                p_index = 1;
            }
        }
    }
     
    pwd_options->L = l_index;
    pwd_options->P = p_index;
}

int count_args_pwd(char **args, int n_options) {
    int num_args = 0;

    for (int i = n_options; args[i] != NULL; i++) {
        num_args++;
    }
    if (num_args > 1 && n_options >= 0) {
        mx_printerr("ush: pwd: too many arguments\n");
    }
    return num_args;
}

void print_pwd(char *dir, int *exit_code, Prompt *shell, pwd_t pwd_options) {
    *exit_code = 0;
    if (pwd_options.P < 0) {
        printf("%s\n", shell->pwd);
    }
    else {
        printf("%s\n", dir);
    }
    free(dir);
}

int mx_pwd(Prompt *shell, Process *p) {
    int n_options = mx_count_options(p->argv, "LP", "pwd", " [-LP]");
    int num_args = count_args_pwd(p->argv, n_options);
    int exit_code = 1;
    char *dir;
    pwd_t pwd_options = {0, 0};

    options_fill_pwd(n_options, &pwd_options, p->argv);

    if (n_options <  0 || num_args > 1) {
        return 1;
    }
    dir = getcwd(NULL, 1024);

    if (dir != NULL) {
        print_pwd(dir, &exit_code, shell, pwd_options);
    }
    else {
        perror("ush: pwd");
    }
    
    return exit_code;
}
