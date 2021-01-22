#include "ush.h"


void mx_err_j(char *arg1, char *arg2, char *arg3, char *arg4) {
    char *error = NULL;
    error = mx_strjoin(error, arg1);
    error = mx_strjoin(error, arg2);
    error = mx_strjoin(error, arg3);
    error = mx_strjoin(error, arg4);
    mx_printerr(error);
    mx_strdel(&error);
}

int mx_check_args(Prompt *shell, Process *proces) {
    int id_job;

    if (proces->argv[1][0] == '%' && isdigit(proces->argv[1][1])) {
        if ((id_job = atoi((proces->argv[1] + 1))) < 1) {
            mx_err_j(proces->argv[0], ": ", proces->argv[1],": no such job\n");
            return -1;
        }
    } else if (proces->argv[1][0] == '%' && !isdigit(proces->argv[1][1])) {
        if ((id_job = mx_g_find_job(shell, (proces->argv[1] + 1))) < 1) {
            mx_err_j(proces->argv[0], ": job not found: ", (proces->argv[1] + 1), "\n");
            return -1;
        }
    } else {
        if ((id_job = mx_g_find_job(shell, proces->argv[1])) < 1) {
            mx_err_j(proces->argv[0], ": job not found: ", proces->argv[1], "\n");
            return -1;
        }
    }
    return id_job;
}

int mx_bg_get_job_id(Prompt *shell, Process *proces) {
    int id_job;
    int args = 0;

    for (int i = 0; proces->argv[i] != NULL; i++) args++;
    if (args > 2) {
        mx_printerr("ush: bg: too many arguments\n"); 
        return -1;
    } else if (args == 1 && (id_job = shell->jobs_stack->last) < 1) {
        mx_printerr("bg: no current job\n");
        return -1;
    } else {
        if ((id_job = mx_check_args(shell, proces)) < 1) return -1;
    }
    return id_job;
}

int mx_bg(Prompt *shell, Process *proces) {
    pid_t p_gid = 0;
    int id_job = 0;

    mx_set_last_job(shell);
    if ((id_job = mx_bg_get_job_id(shell, proces)) < 1)  return 1;

    if ((p_gid = mx_get_pgid_by_job_id(shell, id_job)) < 1) {
        mx_err_j(proces->argv[0], ": ", proces->argv[1],": no such job\n");
        return 1;
    }

    if (kill(-p_gid, SIGCONT) < 0) {
        mx_err_j(proces->argv[0], ": job not found: ", proces->argv[1], "\n");
        return 1;
    }

    mx_set_job_status(shell, id_job, MX_STATUS_CONTINUED);
    mx_print_job_status(shell, id_job, 0);
    return 0;
}
