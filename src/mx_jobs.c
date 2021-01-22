#include "ush.h"

int count_args_jobs(char **args, int n_options) {
    int n_args = 0;

    for (int i = n_options + 1; args[i] != NULL; i++) {
        n_args++;
    }
    return n_args;
}

void options_fill(int n_options, t_jobs *jobs_op, char **args) {
    for (int i = n_options; i > 0; i--) {
        for (int j = mx_strlen(args[i]); j > 0; j--) {
            if (args[i][j] == 's') {
                jobs_op->s = 1;
            }
            if (args[i][j] == 'r') {
                jobs_op->r = 1;
            }
            if (args[i][j] == 'l') {
                jobs_op->l = 1;
            }
        }
    }
}

void print_jobs_by_mask(Prompt *shell, t_jobs jobs_op, int i) {
    if (jobs_op.r && !jobs_op.s) {
        if (shell->jobs[i] != NULL && mx_job_is_running(shell, i)) {
            mx_print_job_status(shell, i, jobs_op.l);
        }
    }
    else if (jobs_op.s && !jobs_op.r) {
        if (shell->jobs[i] != NULL && !mx_job_is_running(shell, i)) {
            mx_print_job_status(shell, i, jobs_op.l);
        }
    }
    else {
        if (shell->jobs[i] != NULL) {
            mx_print_job_status(shell, i, jobs_op.l);
        }
    }
}

void help_jobs(Prompt *shell, Process *p_process, t_jobs jobs_op, int n_opt) {
    int job_id;

    for (int j = n_opt + 1; p_process->argv[j] != NULL; j++) {
        for (int i = shell->max_number_job; i > 0; i--) {
            if (shell->jobs[i] != NULL) {
                if ((job_id = mx_g_find_job(shell, (p_process->argv[j]))) < 1) {
                    mx_err_j(p_process->argv[0], ": job not found: ", p_process->argv[j], "\n");
                    p_process->exit_code = 1;
                    break;
                }
                print_jobs_by_mask(shell, jobs_op, i);
                break;
            }
        }
    }
}

int mx_jobs(Prompt *shell, Process *p_process) {
    t_jobs jobs_op = {0, 0, 0};
    int n_opt;
    int n_args;

    p_process->exit_code = 0;
    if ((n_opt = mx_count_options(p_process->argv, "lrs", "jobs", " [-lrs]")) < 0) {
        p_process->exit_code = 1;
        return p_process->exit_code;
    }

    n_args = count_args_jobs(p_process->argv, n_opt);
    mx_set_last_job(shell);
    options_fill(n_opt, &jobs_op, p_process->argv);

    if (n_args == 0 && n_opt >= 0) {
        for (int i = 0; i < MX_JOBS_NUMBER; i++) {
            print_jobs_by_mask(shell, jobs_op, i);
        }
    }
    else if (n_args) {
        help_jobs(shell, p_process, jobs_op, n_opt);
    }

    return p_process->exit_code;
}

int mx_get_next_job_id(Prompt *shell) {
    for (int i = 1; i < shell->max_number_job + 1; i++) {
        if (shell->jobs[i] == NULL) {
            return i;
        }
    }

    return -1;
}

int mx_insert_job(Prompt *shell, Job *job) {
    int id;

    if ((id = mx_get_next_job_id(shell)) < 0) {
        return -1;
    }

    shell->max_number_job++;
    job->job_id = id;
    shell->jobs[id] = job;
    mx_push_to_stack(shell, id);
    
    return id;
}

void mx_remove_job(Prompt *shell, int job_id) {
    if (shell->jobs[job_id] == NULL || job_id > MX_JOBS_NUMBER) {
        return;
    }

    mx_destroy_jobs(shell, job_id);
    if (job_id == shell->max_number_job) {
        shell->max_number_job--;
    }

    shell->jobs[job_id] = NULL;
    mx_pop_from_stack(shell, job_id);
}

void mx_remove_job_from_panel(Prompt *shell, int job_id) {
    if (job_id > MX_JOBS_NUMBER || shell->jobs[job_id] == NULL) {
        return;
    }

    if (job_id == shell->max_number_job) {
        shell->max_number_job--;
    }

    shell->jobs[job_id] = NULL;
    mx_pop_from_stack(shell, job_id);
}

int mx_job_id_by_pid(Prompt *shell, int pid) {
    Process *p_process;
    int i;

    for (i = 1; i <= shell->max_number_job + 1; i++) {
        if (shell->jobs[i] != NULL) {
            for (p_process = shell->jobs[i]->first_pr; p_process != NULL; p_process = p_process->next) {
                if (p_process->pid == pid) {
                    return i;
                }
            }
        }
    }

    return -1;
}

int mx_set_job_status(Prompt *shell, int job_id, int status) {
    Process *p_process;

    if (job_id > MX_JOBS_NUMBER || shell->jobs[job_id] == NULL) {
        return -1;
    }

    for (p_process = shell->jobs[job_id]->first_pr; p_process != NULL; p_process = p_process->next) {
        if (p_process->status != MX_STATUS_DONE) {
            p_process->status = status;
        }
    }
    return 0;
}

int mx_job_completed(Prompt *shell, int job_id) {
    Process *p_process;

    if (job_id > MX_JOBS_NUMBER || shell->jobs[job_id] == NULL) {
        return -1;
    }

    for (p_process = shell->jobs[job_id]->first_pr; p_process != NULL; p_process = p_process->next) {
        if (p_process->status != MX_STATUS_DONE && p_process->status != MX_STAT_TERMINATED) {
            return 0;
        }
    }
    return 1;
}

int mx_job_is_running(Prompt *shell, int job_id) {
    Process *p_process;
    int status = 0;

    if (job_id > MX_JOBS_NUMBER || shell->jobs[job_id] == NULL) {
        return -1;
    }

    for (p_process = shell->jobs[job_id]->first_pr; p_process != NULL; p_process = p_process->next) {
        if (p_process->status == MX_STATUS_RUNNING) {
            status = 1;
        }
    }
    return status;
}

int mx_g_find_job(Prompt *shell, char *arg) {
    Process *p_process;
    int i;

    for (i = shell->jobs_stack->top; i >= 0 ; i--) {
        int a = shell->jobs_stack->stack[i];
        if (shell->jobs[a] == NULL) {
            continue;
        }
        for (p_process = shell->jobs[a]->first_pr; p_process != NULL; p_process = p_process->next) {
            if ((strncmp(p_process->argv[0], arg, strlen(arg))) == 0) {
                return a;
            }
        }
    }
    return -1;
}

void mx_destroy_jobs(Prompt *shell, int id) {
    Process *p_process;

    for (p_process = shell->jobs[id]->first_pr; p_process != NULL; p_process = p_process->next) {
        mx_clear_process(p_process);
    }
    free(shell->jobs[id]);
}

void mx_check_jobs(Prompt *shell) {
    int job_id;
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, MX_WNOHANG | MX_WUNTRACED | MX_WCONTINUED)) > 0) {
        if (MX_WIFEXITED(status)) {
            mx_set_process_status(shell, pid, MX_STATUS_DONE);
        }
        else if (MX_WIFSTOPP(status)) {
            mx_set_process_status(shell, pid, MX_STATUS_SUSPENDED);
        }
        else if (MX_WIFCONT(status)) {
            mx_set_process_status(shell, pid, MX_STATUS_CONTINUED);
        }

        job_id = mx_job_id_by_pid(shell, pid);

        if (job_id > 0 && mx_job_completed(shell, job_id)) {
            mx_print_job_status(shell, job_id, 0);
            mx_remove_job(shell, job_id);
        }
    }
}

int mx_wait_job(Prompt *shell, int job_id) {
    int wait_count = 0;
    int wait_pid = -1;
    int status = 0;
    int proc_count = mx_get_proc_count(shell, job_id, MX_FILT_IN_PROGR);
    
    while (wait_count < proc_count) {
        wait_count++;
        wait_pid = waitpid(-shell->jobs[job_id]->pgid, &status, MX_WUNTRACED);
        if (MX_WIFEXITED(status)) {
            mx_set_process_status(shell, wait_pid, MX_STATUS_DONE);
        }
        else if (MX_WSTOPSIG(status)) {
            mx_set_process_status(shell, wait_pid, MX_STATUS_SUSPENDED);
            if (wait_count == proc_count) {
                mx_print_job_status(shell, job_id, 0);
            }
        }
        else if (MX_WIFSIGNALED(status)) {
            mx_set_process_status(shell, wait_pid, MX_STAT_TERMINATED);
        }
    }
    return status >> 8;
}



void mx_init_jobs_stack(Prompt *shell) {
    Stack *st = malloc(sizeof(Stack));
    st->size = MX_JOBS_NUMBER;
    st->last = -1;
    st->prev_last = -1;
    st->top = -1;
    st->stack = malloc(sizeof(int) * MX_JOBS_NUMBER);

    for (int i = 0; i < st->size; i++) {
        st->stack[i] = 0;
    }

    shell->jobs_stack = st;
}

void mx_push_to_stack(Prompt *shell, int job) {
    if (shell->jobs_stack->top < shell->jobs_stack->size) {
        shell->jobs_stack->stack[++shell->jobs_stack->top] = job;
    }
}

void mx_pop_from_stack(Prompt *shell, int job) {
    int size = shell->jobs_stack->size;
    int *temp = malloc(sizeof(int) * size);
    int i;
    int j = 0;

    for (i = 0; i < size; i++) {
        temp[i] = shell->jobs_stack->stack[i];
    }

    for (i = 0; i < size; i ++) {
        shell->jobs_stack->stack[i] = 0;
    }

    if (shell->jobs_stack->top >= 0) {
        for (i = 0; j < size; i++, j++) {
            if (temp[i] == job) {
                i++;
            }
            shell->jobs_stack->stack[j] = temp[i];
        }
    }

    shell->jobs_stack->top--;
    free(temp);
}

bool mx_get_froshelltack(Prompt *shell, int job_id) {
    if (shell->jobs_stack->top >= 0) {
        if (job_id == shell->jobs_stack->stack[shell->jobs_stack->top]) {
            return true;
        }
    }
    return false;
}

int mx_get_job_status(Prompt *shell, int job_id, int status) {
    Process *p;
    int flag = 0;

    if (job_id > MX_JOBS_NUMBER || shell->jobs[job_id] == NULL) {
        return -1;
    }
    for (p = shell->jobs[job_id]->first_pr; p != NULL; p = p->next) {
        if (p->status == status) {
            flag = 1;
        }
    }
    return flag;
}

void mx_print_stack(Prompt *shell) {
    int job_id;

    if (shell->jobs_stack->top >= 0) {
        job_id = shell->jobs_stack->stack[shell->jobs_stack->top];
        for (int i = 0; i < shell->jobs_stack->size; i++) {
            printf("%d   ", shell->jobs_stack->stack[i]);
        }
        printf("\n");
    }
}

void mx_set_last_job(Prompt *shell) {
    int last = -1;
    int size = shell->jobs_stack->top;

    for (int i = size; i >= 0; i--) {
        if (mx_get_job_status(shell, shell->jobs_stack->stack[i], 2)) {
            last = shell->jobs_stack->stack[i];
            break;
        }
    }

    if (last == -1) {
        for (int j = size; j >= 0; j--) {
            if ((mx_get_job_status(shell, shell->jobs_stack->stack[j], 0) > 0)
                && (shell->jobs[shell->jobs_stack->stack[j]]->foregrd == 0)) {
                last = shell->jobs_stack->stack[j];
                break;
            }
        }
    }

    shell->jobs_stack->last = last;
}

int mx_get_pgid_by_job_id(Prompt *shell, int job_id) {
    if (job_id > MX_JOBS_NUMBER || shell->jobs[job_id] == NULL) {
        return -1;
    }
    
    return shell->jobs[job_id]->pgid;
}

static void print_spaces(int number) {
    for (int i = 0; i < number; i++) {
        mx_printchar(' ');
    }
}

void mx_print_job_status(Prompt *shell, int job_id, int flag) {
    const char* status[] = {"running", "done", "suspended", "continued", "terminated"};
    int len;
    Process *p;

    printf("[%d] ", job_id);
    if (shell->jobs_stack->last == job_id) {
        printf("%2c ", 43);
    }
    else if (shell->jobs_stack->prev_last == job_id) {
        printf("%2c ", 45);
    }
    else {
        printf("%2c ", ' ');
    }

    for (p = shell->jobs[job_id]->first_pr; p != NULL; p = p->next) {
        if(flag) {
            flag  = printf("%d ", p->pid);
        }
        else {
            flag = printf("");
        }

        printf("%s", status[p->status]);
        len = mx_strlen(status[p->status]);
        print_spaces(MAX_LEN + 1 - len);
        mx_print_args_in_line(p->argv, " ");
        (p->next != NULL) ? mx_printstr(" |\n       ") : mx_printstr("\n");
    }
}

void mx_print_pid_process_in_job(Prompt *shell, int job_id) {
    Process *p_process;

    if (shell->jobs[job_id] == NULL || job_id > MX_JOBS_NUMBER) {
        mx_printstr("error job_id");
        mx_printstr("\n");
    }
    else {
        printf("[%d]", job_id);
        for (p_process = shell->jobs[job_id]->first_pr; p_process != NULL; p_process = p_process->next) {
            printf(" %d", p_process->pid);
        }
        printf("\n");
    }
}

static Job *init_job(Process *first_p) {
    Job *new_job = (Job *) malloc(sizeof(Job));

    new_job->first_pr = first_p;
    new_job->foregrd = 1;

    for (; first_p != NULL; first_p = first_p->next) {
        if (!first_p->foregrd) new_job->foregrd = 0;
    }
    new_job->job_id = -1;
    new_job->pgid = 0;
    new_job->infile = new_job->stdin;
    new_job->stdin = STDIN_FILENO;
    new_job->stdout = STDOUT_FILENO;
    new_job->stderr = STDERR_FILENO;
    new_job->outfile = new_job->stdout;
    new_job->errfile = new_job->stderr;
    return new_job;
}

Job *mx_create_job(Prompt *shell, Abstract *list) {
    Process *first_p = NULL;

    for (Abstract *l = list; l; l = l->next) {
        if ((l->args = mx_filters(l->token, shell)) && *(l->args))
            mx_push_process_back(&first_p, shell, l);
        else {
            mx_clear_process(first_p);
            return NULL;
        }
    }
    return init_job(first_p);
}

void execute_job_env (Job *job) {
    extern char **environ;

    job->env = environ;
    job->path = getenv("PATH");

    if (!job->path) {
        job->path = "";
    }
}

void help_ex_job (Prompt *shell, Job *job, Process *p_process, int job_id) {
    p_process->errfile = job->errfile;
    p_process->outfile = job->outfile;
    p_process->infile = job->infile;

    job->flag = 0;
    if (!p_process->pipe) {
        job->flag = mx_get_flag(p_process->argv);
    }

    if (job->flag) {
        job->exit_code = mx_set_parametr(p_process->argv, shell);
        mx_remove_job(shell, job_id);
    }
    else if (p_process->type != -1) {
        job->exit_code = mx_launch_builtin(shell, p_process, job_id);
    }
    else {
        job->exit_code = mx_launch_process(shell, p_process, job_id);
    }

    if (job->infile != job->stdin) {
        close(job->infile);
    }

    if (job->outfile != job->stdout) {
        close(job->outfile);
    }

    shell->exit_code = job->exit_code;
}

void launch_help (Prompt *shell, Job *job, int job_id, int status) {
    int shell_terminal = STDIN_FILENO;

    if (job->foregrd) {
        tcsetpgrp(STDIN_FILENO, job->pgid);

        if (status > 0) {
            status = mx_wait_job(shell, job_id);
        }
        else if (status == 0) {
            status = mx_wait_job(shell, job_id);
            shell->exit_code = status;
        }
        if (mx_job_completed(shell, job_id)) {
            mx_remove_job(shell, job_id);
        }

        signal(SIGTTOU, MX_SIG_IGN);
        tcsetpgrp(STDIN_FILENO, getpid());
        tcgetattr(shell_terminal, &job->tmodes);
        tcsetattr(shell_terminal, TCSADRAIN, &shell->tmodes);
    }
    else{
        mx_print_pid_process_in_job(shell, job->job_id);
    }
}

void m_pipe(Prompt *shell, Job *job, int mypipe[2], int job_id) {
    if (pipe(mypipe) < 0) {
        perror("pipe");
        mx_remove_job(shell, job_id);
        exit(1);
    }

    job->outfile = mypipe[1];
}

int execute_job (Prompt *shell, Job * job, int job_id) {
    Process *p_process;
    int mypipe[2];

    execute_job_env(job);
    for (p_process = shell->jobs[job_id]->first_pr; p_process; p_process = p_process->next) {
        mx_check_exit(shell, p_process);

        if (p_process->pipe){
            m_pipe(shell, job, mypipe, job_id);
            p_process->r_outfile[0] = job->outfile;
        }

        if ((mx_set_redirections(shell, job, p_process)) != 0) {
            p_process->status = MX_STATUS_DONE;
            p_process->exit_code = 1;
            continue;
        }

        help_ex_job(shell, job, p_process, job_id);
        job->infile = mypipe[0];
    }

    launch_help(shell, job, job_id, job->exit_code);
    
    return job->exit_code;
}

void mx_launch_job (Prompt *shell, Job *job) {
    setbuf(stdout, NULL);

    int status = 0;
    int job_id;

    mx_check_jobs(shell);
    job_id = mx_insert_job(shell, job);
    if (!job->job_type) {
        status = execute_job(shell, job, job_id);
    }
    else if (job->job_type == OR && shell->exit_code != 0) {
        status = execute_job(shell, job, job_id);
    }
    else if (job->job_type == AND && shell->exit_code == 0) {
        status = execute_job(shell, job, job_id);
    }
    else {
        mx_remove_job(shell, job_id);
    }

    !shell->exit_code ? shell->exit_code = status : 0;

    char *exit_status = mx_itoa(shell->exit_code);
    mx_set_variable(shell->variables, "?", exit_status);

    free(exit_status);
}
