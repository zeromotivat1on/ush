#include "ush.h"

static int count_argv(char **args, int n_options) {
    int n_args = 0;

    for (int i = n_options; args[i] != NULL; i++) {
        n_args++;
    }
    return n_args;
}

static int check_identifier(char *arg) {
    int flag = 0;

    for (int j = 0; j < mx_strlen(arg); j++) {
        if (!isalpha(arg[j]) && !isdigit(arg[j]) && arg[j] != '_') {
            flag++;
        }
    }
    return flag;
}

void delete_name(Export **list, char *arg) {
    Export *head = *list;

    if (strcmp(head->name, arg) == 0) {
        *list = (*list)->next;
        return;
    }
    for (;head != NULL;) {
        if (head->next != NULL && strcmp(head->next->name, arg) == 0) {
            free(head->next->name);
            free(head->next->value);
            free(head->next);
            head->next = head->next->next;
            break;
        }
        head = head->next;
    }
}

void unset_or_error(Prompt *shell, char *arg, int *exit_code) {
    int flag = check_identifier(arg);

    if (flag) {
        mx_printerr("ush: unset: `");
        mx_printerr(arg);
        mx_printerr("': not a valid identifier\n");
    }
    else {
        delete_name(&shell->exported, arg);
        delete_name(&shell->variables, arg);
        unsetenv(arg);
        exit_code = 0;
    }
}

int mx_unset(Prompt *shell, Process *p) {
    int exit_code = 0;
    int n_options = mx_count_options(p->argv, "", "unset", " [name ...] ");
    int i = 0;
    int args = count_argv(p->argv, n_options);

    if (n_options <  0 || args == 1) {
        return 1;
    }
    else {
        i = n_options + 1;
        for (;p->argv[i] != NULL;) {
            unset_or_error(shell, p->argv[i], &exit_code);
            i++;
        }
    }
    return exit_code;
}
