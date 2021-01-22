#include "ush.h"

int kill_count_args(char **args) {
    int num_args = 0;

    for (int i = 1; args[i] != NULL; i++) {
        num_args++;
    }

    return num_args;
}

int kill_check_pid(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!(mx_isdigit(str[i]))) {
            return 1;
        }
    }
    return 0;
}

void kill_job(Prompt *shell, Process *p_process, int i) {
    int pgid;
    int job_id;

    if ((job_id = mx_check_args(shell, p_process)) < 1) {
        p_process->exit_code = 1;
        return;
    }

    if ((pgid = mx_get_pgid_by_job_id(shell, job_id)) < 1) {
        mx_err_j(p_process->argv[0], ": ", p_process->argv[i],": no such job\n");
        p_process->exit_code = 1;
        return;
    }

    if (kill(-pgid, SIGTERM) < 0) {
        mx_err_j(p_process->argv[0], ": job not found: ", p_process->argv[i], "\n");
        p_process->exit_code = 1;
    }

    mx_set_job_status(shell, job_id, MX_STAT_TERMINATED);
}

void kill_pid(Prompt *shell, Process *p_process, int i) {
    pid_t pid = 0;

    if ((kill_check_pid(p_process->argv[i])) > 0) {
        mx_err_j(p_process->argv[0], ": illegal pid: ", p_process->argv[i], "\n");
        p_process->exit_code = 1;
        return;
    }
    else {
        pid = atoi(p_process->argv[i]);
        if (kill(pid, SIGTERM) < 0) {
            mx_err_j(p_process->argv[0], ": kill ", p_process->argv[i], " failed: no such process\n");
            p_process->exit_code = 1;
        }
        mx_set_process_status(shell, pid, MX_STAT_TERMINATED);
    }
}

int mx_kill(Prompt *shell, Process *p_process) {
    int n_args = 0;
    p_process->exit_code = 0;

    n_args = kill_count_args(p_process->argv);
    mx_set_last_job(shell);

    if (n_args == 0) {
        mx_printerr("ush: kill: not enough arguments\n");
        p_process->exit_code = 1;
    }

    if (n_args) {
        for (int i = 1; i <= n_args; i++) {
            if (p_process->argv[i][0] == '%')
                kill_job(shell, p_process, i);
            else
                kill_pid(shell, p_process, i);
        }
    }

    return p_process->exit_code;
}
