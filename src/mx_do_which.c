#include "ush.h"

void fill_options(int options, which_t *w_options, char **args) {
    for(int i = options; i > 0; i --) {
        for (int j = mx_strlen(args[i]); j > 0; j--) {
            if (args[i][j] == 's') 
                w_options->s = 1;
            if (args[i][j] == 'a') 
                w_options->a = 1;
        }
    }
}

int mx_which(Prompt *m_s, Process *p) {
    which_t w_options = {0, 0};
    int options = mx_count_options(p->argv, "as",
                                     "which", " [-as] program ...");
    int exit_code = 0;
    int flag = 0;
    
    fill_options(options, &w_options, p->argv);
    if (options < 0)
        return 1;
    for (int i = options + 1; p->argv[i] != NULL; i++) {
        mx_get_command_info(m_s, p->argv[i], &flag, w_options);
        if (!flag)
            exit_code = 1;
    }
    return exit_code;
}

void check_path(char **arr, char *command, t_list **output,int *flag) {
    DIR *dir;
    struct dirent *ds;
    char *name = NULL;
    int i = 0;
    
    while (arr[i] != NULL) {
        dir = opendir(arr[i]);
        if (dir != NULL) {
            for (;(ds = readdir(dir)) != 0;) {
                if (strcmp(ds->d_name, command) == 0 && command[0] != '.') {
                    name = strcat(arr[i], "/");
                    name = strcat(name, command);
                    mx_push_back(&*output, name);
                    (*flag)++;
                }
            }
            closedir(dir);
        }
        i++;
    }
}

void check_builtin (char **list, char *command, t_list **output, int *flag) {
    for (int j = 0; list[j] != NULL; j++) {
        if (strcmp(list[j], command) == 0) {
            char *str = mx_strjoin(command, ": ush built-in command");
            mx_push_back(&*output, str);
            free(str);
            (*flag)++;
        }
    }
}

void print_path(t_list *output, int flag, char *command, which_t options) {
    if (!flag){
        mx_printerr(command);
        mx_printerr(" not found\n");
    }
    else {
        t_list *head = output;

        for (;head;) {
            printf("%s\n", head->data);
            if (!options.a)
                break;
            head = head->next;
        }
    }
}

void mx_clear_list1(t_list **list) {
    t_list *tmp = *list;
    t_list *temp = NULL;

    if (!(*list) || !list)
        return;
    for (;tmp;) {
        mx_strdel((char **)&tmp->data);
        temp = tmp->next;
        free(tmp);
        tmp = temp;
    }
    *list = NULL;
}

void mx_get_command_info(Prompt *shell, char *command, int *flag, which_t options) {
	t_list *output= NULL;
    char *path = getenv("PATH");
    char **arr = NULL;

    if (!path)
        path = "";
	arr = mx_strsplit(path, ':');
	*flag = 0;
	check_builtin(shell->builtin_list, command, &output, flag);
    check_path(arr, command, &output, flag);
    mx_del_strarr(&arr);
    if (!options.s)
        print_path(output, *flag, command, options);
    mx_clear_list1(&output);
}
