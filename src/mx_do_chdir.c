#include "ush.h"

int mx_chdir(Prompt *shell, Process *p) {
    cd_t cd_options = {0, 0, 0};
    int exit_point = 1;
    int option_num = mx_count_options(p->argv, "sLP", "chdir", " [-s] [-L|-P] [dir]");
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
