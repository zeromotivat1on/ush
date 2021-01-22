#include "ush.h"

static int fg_send_signal(Prompt *shell, Process *proc, int pg_id, int id_job) {
    if (kill(-pg_id, SIGCONT) < 0) {
        mx_err_j("fg", ": job not found: ", proc->argv[1], "\n");
        return 1;
    }

    int stat;
    tcsetpgrp(STDIN_FILENO, pg_id);
    mx_set_job_status(shell, id_job, MX_STATUS_CONTINUED);
    mx_print_job_status(shell, id_job, 0);
    stat = mx_wait_job(shell, id_job);
    if (mx_job_completed(shell, id_job)) mx_remove_job(shell, id_job);
    signal(SIGTTOU, MX_SIG_IGN);
    tcsetpgrp(STDIN_FILENO, getpid());
    signal(SIGTTOU, MX_SIG_DFL);
    tcgetattr(STDIN_FILENO, &shell->jobs[id_job]->tmodes);
    tcsetattr(STDIN_FILENO, TCSADRAIN, &shell->jobs[id_job]->tmodes);
    return stat;
}

static int fg_get_job_id (Prompt *shell, Process *proc) {
    int id_job, args_num = 0;

    for (int i = 0; proc->argv[i] != NULL; i++) args_num++;
    if (args_num > 2) {
        mx_printerr("ush: fg: too many arguments\n");
        return -1;
    } else if (args_num == 1) {
        if ((id_job = shell->jobs_stack->last) < 1) {
            mx_printerr("fg: no current job\n");
            return -1;
        }
    } else {
        if ((id_job = mx_check_args(shell, proc)) < 1) return -1;
    }
    return id_job;
}

int mx_fg(Prompt *shell, Process *proc) {
    int stat, id_job = 0;;
    pid_t pg_id = 0;

    mx_set_last_job(shell);
    if ((id_job = fg_get_job_id(shell, proc)) < 1) return 1;

    if ((pg_id = mx_get_pgid_by_job_id(shell, id_job)) < 1) {
        mx_err_j(proc->argv[0], ": ", proc->argv[1],": no such job\n");
        return 1;
    }
    
    stat = fg_send_signal(shell, proc, pg_id, id_job);
    return stat;
}
