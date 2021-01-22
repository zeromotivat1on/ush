#include "ush.h"

void read_dir(char *dir_name, char *command, int *flag, char **name_arr) {
    struct dirent  *ds;
    DIR *dptr = opendir(dir_name);

    if (dptr != NULL) {
        for (;(ds = readdir(dptr)) != 0;) {
            if (strcmp(ds->d_name, command) == 0 && command[0] != '.') {
                (*flag)++;
                char *temp = mx_strjoin(dir_name, "/");
                *name_arr = mx_strjoin(temp, command);
                free(temp);
                break;
            }
        }
        closedir(dptr);
    }
}

char *check_path_launch_process(char **arr_str, char *command) {
    int i = 0;
    int flag = 0;
    char *name = NULL;

    while (arr_str[i] != NULL && !flag) {
        read_dir(arr_str[i], command, &flag, &name);
        i++;
    }
    return name;
}

char *error_getter(char **name_arr, char *command, int *status) {
    *status = 127;
    char *error = NULL;

    if (strstr(command, "/")) {
        struct stat buff;
        *name_arr = command;

        if (lstat(*name_arr, &buff) < 0) {
            error = NULL;
        }
        else {
            if (mx_get_type(buff) == 'd') {
                error = strdup("is a directory: ");
                *status = 126;
            }
        }
    }
    else {
        error = strdup("command not found: ");
    }
    
    return error;
}

char *error_getter_bin(char **name, char *command, int *status) {
    char *error = NULL;

    *status = 127;
    if (strstr(command, "/")) {
        struct stat buff;
        *name = command;
        if (lstat(*name, &buff) < 0) {
            error = NULL;
        }
        else {
            if (mx_get_type(buff) == 'd') {
                error = strdup(": is a directory\n");
                *status = 126;
            }
        }
    }
    return error;
}

static void child_wrk(Prompt *shell, Process *p_process, int job_id, int child_pid) {
    int shell_is_interactive = isatty(STDIN_FILENO);

    if (shell_is_interactive) {
        mx_pgid(shell, job_id, child_pid);
    }

    mx_dup_fd(p_process);
    char *command = p_process->argv[0];
    char **arr = mx_strsplit(shell->jobs[job_id]->path, ':');
    shell->jobs[job_id]->path  = check_path_launch_process(arr, command);
    mx_del_strarr(&arr);
    char *error = error_getter(&shell->jobs[job_id]->path, command, &p_process->status);

    if(execve(shell->jobs[job_id]->path, p_process->argv, shell->jobs[job_id]->env) < 0) {
        mx_print_error(error, command);
        mx_printerr("\n");
        free(error);
        free(shell->jobs[job_id]->path);
        _exit(p_process->status);
    }

    free(shell->jobs[job_id]->path);
    free(error);

    exit(p_process->status);
}

int mx_launch_process(Prompt *shell, Process *p_process, int job_id) {
    int shell_is_interactive = isatty(STDIN_FILENO);
    pid_t child_pid;

    p_process->status = MX_STATUS_RUNNING;
    child_pid = fork();

    if (child_pid < 0) {
        perror("error fork");
        exit(1);
    }
    else if (child_pid == 0) {
        child_wrk(shell, p_process, job_id, child_pid);
    }
    else {
        p_process->pid = child_pid;
        if (shell_is_interactive) {
            pid_t pid = child_pid;
            if (shell->jobs[job_id]->pgid == 0) {
                shell->jobs[job_id]->pgid = pid;
            }
            setpgid (pid, shell->jobs[job_id]->pgid);
        }
    }
    
    return p_process->status >> 8;
}

static Process *init_process(Abstract *list) {
    Process *process = (Process *)malloc(sizeof(Process));

    if (!process) {
        return NULL;
    }

    process->argv = mx_strdup_arr(list->args);
    process->delim = list->type;
    process->command = mx_strdup(list->args[0]);
    process->input_path = NULL;
    process->output_path = NULL;
    process->redir_delim = 0;
    process->redirect = NULL;
    process->foregrd = 1;
    process->pipe = 0;
    process->r_infile = (int *) malloc(sizeof(int) * 1);
    process->r_outfile = (int *) malloc(sizeof(int) * 1);

    if (process->delim == FON) {
        process->foregrd = 0;
    }
    else if (process->delim == PIPE) {
        process->pipe = 1;
    }

    process->next = NULL;

    return process;
}

static Process *create_process(Prompt *shell, Abstract *list) {
    Abstract *t = list->left;
    Process *process;
    int index = 0;

    if (!(process = init_process(list))) {
        return NULL;
    }

    if (list->left && (t->args = mx_filters(t->token, shell)) && *(t->args)) {
        process->redir_delim = t->type;

        if (MX_IS_REDIR_INP(t->type)) {
            process->input_path = mx_strdup(t->args[0]);
        }
        else if (MX_IS_REDIR_OUTP(t->type)) {
            process->output_path = mx_strdup(t->args[0]);
        }

        for (Abstract *q = list->left; q; q = q->next) {
            if (q->args || ((q->args = mx_filters(q->token, shell)) && *(q->args))) {
                mx_redir_push_back(&process->redirect, q->args[0], q->type);
            }
        }
    }

    if ((index = mx_builtin_commands_idex(shell, process->argv[0])) == -1) {
        process->type = -1;
    }
    else {
        process->type = index;
    }

    return process;
}

void mx_push_process_back(Process **process, Prompt *shell, Abstract *list) {
    Process *tmp;
    Process *process_tmp;

    if (!process || !shell || !list)
        return;
    tmp = create_process(shell, list);
    if (!tmp) {
        return;
    }
    process_tmp = *process;

    if (*process == NULL) {
        *process = tmp;
        return;
    }
    else {
        while (process_tmp->next != NULL) {
            process_tmp = process_tmp->next;
        }
        process_tmp->next = tmp;
    }
}

void mx_clear_process(Process *process) {
    if (!process)
        return;
    mx_del_strarr(&process->argv);
    mx_strdel(&process->command);
    mx_strdel(&process->input_path);
    mx_strdel(&process->output_path);
    mx_redir_clear_list(&process->redirect);
    if (process->r_infile)
        free(process->r_infile);
    if (process->r_outfile)
        free(process->r_outfile);
    free(process);
}

char *check_path_launch_process_bin(char **arr_str, char *command) {
    int flag = 0;
    char *name = NULL;

    for (int i = 0; arr_str[i] != NULL && !flag; i++) {
        DIR *dptr  = opendir(arr_str[i]);
        if (dptr != NULL) {
            struct dirent  *ds;
            while ((ds = readdir(dptr)) != 0) {
                if (command[0] != '.' && strcmp(ds->d_name, command) == 0) {
                    flag++;
                    name = mx_strjoin(arr_str[i], "/");
                    name = mx_strjoin(name, command);
                    break;
                }
            }
            closedir(dptr);
        }
    }
    return name;
}

void print_error_env(char *command, char *error) {
    mx_printerr("env: ");

    if (error) {
        mx_printerr(command);
        mx_printerr(error);
        free(error);
    }
    else {
        perror(command);
    }
}

void child_process(Process *p_process, char *path, char **env, int *status_arr) {
    char *const *envp =  env;
    char *command = p_process->argv[0];
    char **arr = mx_strsplit(path, ':');
    char *error;

    path  = check_path_launch_process_bin(arr, command);
    if(!path) {
        path = strdup(command);
    }

    error = error_getter_bin(&path, command, status_arr);

    if (execve(path, p_process->argv, envp) < 0) {
        print_error_env(command, error);
        _exit(*status_arr);
    }

    exit(0);
}

int mx_launch_bin(Process *p_process, char *path, char **env) {
    pid_t pid;
    int status = 1;

    pid = fork();
    if (pid == 0) {
        child_process(p_process, path, env, &status);
    }
    else if (pid < 0) {
        perror("env ");
    } 
    else {
        waitpid(pid, &status, 0);
    }

    return status >> 8;
}

void mx_dup_fd(Process *p_process) {
    
    if (p_process->errfile != STDERR_FILENO) {
        dup2(p_process->errfile, STDERR_FILENO);
        close(p_process->errfile);
    }

    if (p_process->r_infile[0] != STDIN_FILENO) {
        dup2(p_process->r_infile[0], STDIN_FILENO);
        close(p_process->r_infile[0]);
    }

    if (p_process->r_outfile[0] != STDOUT_FILENO) {
        dup2(p_process->r_outfile[0], STDOUT_FILENO);
        close(p_process->r_outfile[0]);
    }
    
}

void mx_dup_close(int input, int output) {
    if (input != output) {
        dup2(input, output);
        close(input);
    }
}

void mx_dup2_fd(int *fd1, int *fd2) {
    if (fd1[0] != STDIN_FILENO) {
        if (dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
            perror("error dup2 stdin");
        close(fd1[0]);
    }

    if (fd2[1] != STDOUT_FILENO) {
        if (dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
            perror("error dup2 stdout");
        close(fd2[1]);
    }
}

void mx_print_fd(Process  *p_process) {
    printf("\x1B[32m p->r_input \x1B[0m\t");

    for(int i = 0; i < p_process->c_input; i ++) {
        printf("\x1B[32m [%d] \x1B[0m  \t", p_process->r_infile[i]);
    }

    printf("\n");
    printf("\x1B[32m p->r_output \x1B[0m\t");

    for(int i = 0; i < p_process->c_output; i ++) {
        printf("\x1B[32m [%d] \x1B[0m  \t", p_process->r_outfile[i]);
    }
    
    printf("\n");
}
