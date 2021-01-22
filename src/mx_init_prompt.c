#include "ush.h"

void set_path(Prompt *shell) {
    char *path = NULL;
    char *dir = getcwd(NULL, 256);
    shell->kernal = mx_strjoin(dir, "/ush");

    if (!getenv("PATH")) {
        path = strdup("/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:");
        path = mx_strjoin_free(path, "/usr/local/munki");
    }
    else  {
        path = strdup(getenv("PATH"));
    }

    setenv("PATH", path, 1);

    free(dir);
    free(path);
}

void set_shell_defaults(Prompt *shell) {
    char *b_list[19] = {"env", "export", "unset", "echo", "jobs", "fg", "bg",
                        "cd", "pwd", "which", "exit", "set", "kill", "chdir",
                        "true", "alias", "declare", "false", NULL};
    shell->builtin_list = (char **) malloc(sizeof(char *) * 19);

    for (int i = 0; i < 19; i++) {
        shell->builtin_list[i] = b_list[i]; 
    }
    
    shell->exit_flag = 0;
    shell->history_count = 0;
    shell->max_number_job = 1;
    shell->history_size = 1000;
    shell->history = (char **)malloc(sizeof(char *) * shell->history_size);

    for (int i = -1; i < MX_JOBS_NUMBER; ++i) {
        shell->jobs[i] = NULL;
    }
    
    shell->aliases = NULL;
    shell->functions = NULL;

    mx_init_jobs_stack(shell);
    set_path(shell);
}

char *get_pwd() {
    char *pwd = getenv("PWD");
    char *cur_dir = getcwd(NULL, 256);
    char *read_link = realpath(pwd, NULL);

    if (strcmp(cur_dir, read_link) == 0 && read_link){
        pwd = strdup(getenv("PWD"));
        free(cur_dir);
        free(read_link);
    }
    else {
        pwd = strdup(cur_dir);
        free(cur_dir);
    }

    return pwd;
}

Prompt *init_ush(int argc, char **argv) {
    char *shlvl;
    Prompt *shell = (Prompt *) malloc(sizeof(Prompt));

    set_shell_defaults(shell);
    shell->argv = argv;
    shell->argc = argc;
    shell->pwd = get_pwd();
    setenv("OLDPWD", shell->pwd, 1);
    setenv("PWD", shell->pwd, 1);
    shlvl = mx_get_shlvl();
    setenv("SHLVL", shlvl, 1);
    free(shlvl);
    shell->variables = mx_set_variables();
    shell->prompt_status = 1;
    shell->exported = mx_set_export();
    shell->prompt = strdup("u$h");
    mx_set_shell_grp(shell);
    mx_set_variable(shell->variables, "PROMPT", "u$h");
    shell->exit_code = -1;
    
    return shell;
}

void shell_grp_help(Prompt *shell, pid_t shell_pgid) {
    shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        perror("Couldn't put the shell in its own process group");
        exit(1);
    }
    tcsetpgrp(STDIN_FILENO, shell_pgid);
    shell->shell_pgid = shell_pgid;
}

void mx_set_shell_grp(Prompt *shell) {
    pid_t shell_pgid;
    int shell_terminal = STDIN_FILENO;
    int shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp())) {
            kill(-shell_pgid, SIGTTIN);
        }
        signal(SIGTTOU, MX_SIG_IGN);
        signal(SIGQUIT, MX_SIG_IGN);
        signal(SIGTSTP, MX_SIG_IGN);
        signal(SIGINT, MX_SIG_IGN);
        signal(SIGTTIN, MX_SIG_IGN);

        shell_grp_help(shell, shell_pgid);
        char *c_shell_pgid = mx_itoa(shell->shell_pgid);
        mx_set_variable(shell->variables, "$", c_shell_pgid);
        free(c_shell_pgid);
        
        tcgetattr(shell_terminal, &shell->t_original);
        tcgetattr(shell_terminal, &shell->tmodes);
    }
}

static void k_backscape(int *position, char *line) {
    if (*position > 0) {
        for (int i = *position - 1; i < mx_strlen(line); i++) line[i] = line[i + 1];
        (*position)--;
    }
}

static void k_up(Prompt *shell, char **line, int *position) {
    if (shell->history[shell->history_index - 1] && shell->history_index > 0) {
        free(*line);
        *line = NULL;
        *line = mx_strnew(1024);
        strcpy(*line, shell->history[shell->history_index - 1]);
        *position = mx_strlen(*line);
        shell->history_index--;
    }
}

static void k_down(Prompt *shell, char **line, int *position) {
    if (shell->history[shell->history_index + 1] && shell->history_index < shell->history_count) {
        free(*line);
        *line = NULL;
        *line = mx_strnew(1024);
        mx_strcpy(*line, shell->history[shell->history_index + 1]);
        *position = mx_strlen(*line);
        shell->history_index++;
    }
}

void mx_edit_command(int keycode, int *position, char **line, Prompt *shell) {
    if (keycode == MX_K_LEFT){
        if(*position > 0) (*position)--;
    } else if (keycode == MX_K_RIGHT) {
        if(*position < mx_strlen(*line)) (*position)++;
    } else if (keycode == MX_K_END) {
        *position = mx_strlen(*line);
    } else if (keycode == MX_K_HOME) {
        *position = 0;
    } else if (keycode == MX_K_DOWN || keycode == MX_P_DOWN) {
        k_down(shell, line, position);
    } else if (keycode == MX_K_UP || keycode == MX_P_UP) {
        k_up(shell, line, position);
    } else if (keycode == MX_C_PROMPT) {
        if(shell->prompt_status)  shell->prompt_status--;
        else shell->prompt_status++;
        mx_edit_prompt(shell);
    } else if (keycode == MX_BACKSCAPE){
        k_backscape(position, *line);
    }
}

int mx_builtin_commands_idex(Prompt *shell, char *command) {
    int i = 0;

    for (i = 0; shell->builtin_list[i] != NULL; i++) {
        if (strcmp(command, shell->builtin_list[i]) == 0)
            return (i);
    }
    return (-1);
}

int mx_get_flag(char **args) {
    int flag = 1;

    for (int i = 0; args[i] != NULL; i++) {
        if (mx_get_char_index(args[i],'=') <= 0) {
            flag--;
            break;
        }
    }
    return flag;
}

void mx_check_exit(Prompt *shell, Process *p) {
    mx_set_variable(shell->variables, "_", p->argv[0]);
    setenv("_", p->argv[0], 1);
    if (shell->exit_flag == 1 && !(p->type == 10))
        shell->exit_flag = 0;
}

char *mx_get_shlvl(void) {
    char *shlvl = NULL;
    int lvl;

    shlvl = getenv("SHLVL");
    if (!shlvl)
        shlvl = "0";
    lvl = atoi(shlvl);
    lvl++;
    shlvl = mx_itoa(lvl);
    return shlvl;
}
static void exit_ush(Prompt *shell) {
    printf("exit\n");
    mx_clear_all(shell);
    exit(EXIT_SUCCESS);
}

void mx_exec_signal(int keycode, char **line, int *position, Prompt *shell) {
    if (keycode == MX_CTRL_C)
        for (int i = 0; i < mx_strlen(*line); i++) *line[i] = '\0';

    if (keycode == MX_CTRL_D){
        if (strcmp(*line, "") == 0) 
            exit_ush(shell);
        else
            for (int i = *position; i < mx_strlen(*line); i++) line[i] = line[i + 1];
    } else if (keycode == MX_TAB) {
        if (mx_strlen(*line) == 0) return;

        static DIR *dir = NULL;
        static struct dirent *entry = NULL;
        static int tmp_pos = 0;
        static int length_tab_word = 0;

        if (*position < tmp_pos) {
            if (mx_strlen(*line) == 0 || !mx_isspace((*line)[*position - 1])) return;
            tmp_pos = *position;
            length_tab_word = 0;
        }
        if (dir == NULL) dir = opendir(".");
        if (dir == NULL) return;

        if (tmp_pos == 0 || *position > tmp_pos + length_tab_word) {
            tmp_pos = *position;
            closedir(dir);
            dir = opendir(".");
        }

        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                for (int i = tmp_pos; i <= *position; i++) (*line)[i] = '\0';
                *position = tmp_pos;
                length_tab_word = mx_strlen(entry->d_name);
                for (int i = 0; i < length_tab_word; i++) (*line)[(*position)++] = entry->d_name[i];
                return;
            }
        }
        closedir(dir);
        dir = NULL;
    }
}

void buildin_fork(Prompt *shell, int job_id, int (*builtin_functions[]) (Prompt *shell, Process *p_process), Process *p_process) {
    pid_t child_pid = fork();

    p_process->pid = child_pid;
    if (child_pid < 0) {
        perror("error fork");
        exit(1);
    }
    else if (child_pid == 0) {
        if (isatty(STDIN_FILENO)) {
            mx_pgid(shell, job_id, child_pid);
        }
        mx_dup_fd(p_process);
        p_process->exit_code = builtin_functions[p_process->type](shell, p_process);
        exit(p_process->exit_code);
    }
    else {
        if (isatty(STDIN_FILENO)) {
            if (shell->jobs[job_id]->pgid == 0) {
                shell->jobs[job_id]->pgid = child_pid;
            }
            setpgid (child_pid, shell->jobs[job_id]->pgid);
        }
    }
}

void buildin_std_ex(Prompt *shell, int (*builtin_functions[]) (Prompt *shell, Process *p_process), Process *p_process) {
    int defoult;

    if (p_process->output_path) {
        defoult = dup(1);
        if (p_process->outfile != STDOUT_FILENO) {
            lseek(p_process->outfile, 0, SEEK_END);
            dup2(p_process->outfile, STDOUT_FILENO);
            close(p_process->outfile);
        }
        mx_dup_close(p_process->infile, STDIN_FILENO);
    }

    p_process->exit_code = builtin_functions[p_process->type](shell, p_process);
    p_process->status = MX_STATUS_DONE;

    if (p_process->output_path) {
        if (p_process->outfile != STDOUT_FILENO) {
            dup2(defoult, 1);
            close(defoult);
        }
    }
}

int mx_launch_builtin(Prompt *shell, Process *p_process, int job_id) {
    int (*builtin_functions[])(Prompt *shell, Process *p_process) =
         {&mx_env, &mx_export, &mx_unset, &mx_echo, &mx_jobs, &mx_fg,
          &mx_bg, &mx_cd, &mx_pwd, &mx_which, &mx_exit, &mx_set,
          &mx_kill, &mx_chdir, &mx_false, &mx_alias, &mx_declare, 
          &mx_true, NULL};

    p_process->status = MX_STATUS_RUNNING;
    if (!p_process->foregrd || p_process->pipe) {
        buildin_fork(shell, job_id, builtin_functions, p_process);
    }
    else {
        buildin_std_ex(shell, builtin_functions, p_process);
    }

    return p_process->exit_code;
}

void mx_pgid(Prompt *shell, int job_id, int child_pid) {
    if (shell->jobs[job_id]->pgid == 0) {
        shell->jobs[job_id]->pgid = child_pid;
    }

    setpgid(child_pid, shell->jobs[job_id]->pgid);

    if (shell->jobs[job_id]->foregrd) {
        tcsetpgrp(STDIN_FILENO, shell->jobs[job_id]->pgid);
    }

    signal(SIGINT, MX_SIG_DFL);
    signal(SIGQUIT, MX_SIG_DFL);
    signal(SIGTSTP, MX_SIG_DFL);
    signal(SIGTTIN, MX_SIG_DFL);
    signal(SIGTTOU, MX_SIG_DFL);
    signal(SIGPIPE, mx_sig_h);
}

char *get_variable(Prompt *shell, char *target) {
    Export *export_head = shell->variables;

    while (export_head != NULL) {
        if (strcmp(export_head->name, target) == 0) {
            return export_head->value;
        }
        export_head = export_head->next;
    }

    return NULL;
}

void customize(Prompt *shell) {
    char **arr_char = NULL;
    char *info_char = NULL;
    int count = 0;

    if (strcmp(shell->pwd, "/") == 0) {
        info_char = strdup("/");
    }
    else if (getenv("HOME") && strcmp(shell->pwd, getenv("HOME")) == 0) {
        info_char = strdup("~");
    }
    else {
        arr_char = mx_strsplit(shell->pwd, '/');
        count = 0;
        while (arr_char[count] != NULL)
            count++;
        info_char = strdup(arr_char[count - 1]);
        mx_del_strarr(&arr_char);
    }

    shell->prompt = strdup(info_char);
    free(info_char);
}

void mx_print_prompt(Prompt *shell) {
    printf("%s", "\x1B[37m");

    if (!shell->prompt_status) {
        printf("%s", MX_BOLD_MAGENTA);
    }

    printf ("%s", shell->prompt);

    if (!shell->prompt_status && shell->git) {
        printf(" %sgit:(%s%s%s)", MX_BOLD_BLUE, MX_RED, shell->git, MX_BOLD_BLUE);
    }

    printf ("> ");
    printf("%s", MX_RESET);
    fflush (NULL);
}

void mx_edit_prompt(Prompt *shell) {
    if (shell->prompt) {
        free(shell->prompt);
    }
    if (!shell->prompt_status) {
        customize(shell);
    }
    else {
        if (get_variable(shell, "PROMPT")) {
            shell->prompt = strdup(get_variable(shell, "PROMPT"));
        }
        else {
            shell->prompt = strdup("u$h");
        }
    }
}

void mx_termios_restore(Prompt *shell) {
    if (shell->custom_terminal == TRUE) {
        tcsetattr(STDIN_FILENO, TCSANOW, &shell->t_original);
    }
}

void mx_termios_save(Prompt *shell) {
    if (tcgetattr(STDIN_FILENO, &shell->t_original) == -1) {
        mx_printerr("tcgetattr() failed");
        exit(MX_EXIT_FAILURE);
    }
    shell->t_custom = shell->t_original;
    shell->t_custom.c_lflag &= ~(ECHO);
    shell->t_custom.c_lflag &= ~(ICANON);
    shell->t_custom.c_cc[VTIME] = 0;
    shell->t_custom.c_cc[VMIN] = 1;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &shell->t_custom) == -1) {
        mx_printerr("tcsetattr() failed");
        exit(MX_EXIT_FAILURE );
    }
    shell->custom_terminal = TRUE;
}

static bool check_subsut_result(char **result, char **args, int *i) {
    if (!result[*i]) {
        mx_del_strarr(&result);
        free(args);
        return true;
    } else if (result[*i][0] == '\0') {
        mx_strdel(&result[*i]);
        (*i)--;
    }
    return false;
}

static char **func_alias_tokens(char *line, Prompt *shell) {
    char **args = NULL;

    if (mx_strstr(line, "()")) {
        if (mx_get_functions(line, shell)) return NULL;
    }

    if (!mx_strncmp(line, "alias", 5) && line[5] && line[5] == ' '
        && line[6] && !mx_isdelim(line[6], MX_USH_TOK_DELIM)) {
        mx_get_aliases(line, shell);
        return NULL;
    }
    args = mx_parce_tokens(line);
    return args;
}

static char **substitutions(char **args, Prompt *shell) {
    int indx = 0;
    char **result = NULL;

    result = (char **)malloc((mx_strlen_arr(args) + 1) * sizeof(char *));
    for (int j = 0; args[j] && args[j][0]; indx++, j++) {
        result[indx] = mx_strdup(args[j]);
        result[indx] = mx_subst_tilde(result[indx], shell->variables);
        result[indx] = mx_substr_dollar(result[indx], shell->variables);
        result[indx] = mx_sub_str_command(result[indx], shell);
        if (check_subsut_result(result, args, &indx)) return NULL;
    }
    result[indx] = NULL;
    return result;
}

char **mx_filters(char *arg, Prompt *shell) {
    char **args;
    char **result = NULL;

    if (!arg) return NULL;

    if ((args = func_alias_tokens(arg, shell))) {
        if (!(result = substitutions(args, shell))) {
            mx_strdel(&arg);
            return NULL;
        }
        mx_strtrim_quote(result);
        free(args);
    }
    return result;
}

char *mx_subs_output(char **result) {
    char *tokens = NULL;
    char *token;
    size_t len_token;
    size_t sum_len = 0;

    token = mx_strtok(*result, MX_USH_TOK_DELIM);

    for (;token != NULL;) {
        len_token = strlen(token);
        if (sum_len == 0) {
            tokens = realloc(tokens, len_token + 1);
            strcpy(tokens, token);
            sum_len += (len_token + 1);
        }
        else {
            tokens = realloc(tokens, sum_len + len_token + 3);
            strcat((tokens), " ");
            strcat((tokens), token);
            sum_len += (len_token + 3);
        }
        token = mx_strtok(NULL, MX_USH_TOK_DELIM);
    }

    if (tokens)
        tokens[sum_len] = '\0';

    mx_strdel(result);
    return tokens;
}

char *subshell_parent(Prompt *shell, int *fd1, int *fd2, int pid) {
    int status;
    size_t n_read = 0;
    char buf[BUFSIZ];
    char *result = mx_strnew(1);
    size_t sum_read = 0;

    close(fd1[0]);
    close(fd2[1]);

    while ((n_read = read(fd2[0], buf, BUFSIZ)) > 0) {
        result = realloc(result, sum_read + n_read + 1);
        memcpy(&result[sum_read], buf, n_read);
        sum_read += n_read;
    }

    if (sum_read > 0) {
        if (result[sum_read - 1] == '\n')
            result[sum_read - 1] = '\0';
    }

    result[sum_read] = '\0';
    waitpid(pid, &status, MX_WNOHANG | MX_WUNTRACED | MX_WCONTINUED);
    shell->exit_code = status;
    close(fd2[0]);
    
    return mx_subs_output(&result);
}

char *exec_sub_shell (Prompt *shell, int *fd1, int *fd2) {
    extern char **environ;
    pid_t pid;
    char *result = NULL;

    if ((pid = fork()) < 0) {
        perror("error fork");
    }
    else if (pid <= 0) {
        mx_dup2_fd(fd1, fd2);
        if (execve(shell->kernal, NULL, environ) < 0) {
            perror("ush ");
            _exit(EXIT_SUCCESS);
        }
        exit(EXIT_SUCCESS);
    }
    else {
        result = subshell_parent(shell, fd1, fd2, pid);
    }
    return result;
}


char *mx_run_sub_shell(char *substr, Prompt *shell) {
    int len;
    int fd1[2];
    int fd2[2];

    if (pipe(fd1) < 0 || pipe(fd2) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    len = strlen(substr) + 1;
    if (write(fd1[1], substr, len) != len)
        perror("error write to pipe");

    close(fd1[1]);
    mx_strdel(&substr);
    char *result = exec_sub_shell(shell, fd1, fd2);

    return result;
}

int mx_get_proc_count(Prompt *shell, int job_id, int filter) {
    Process *p_process;
    int count = 0;

    if (job_id > MX_JOBS_NUMBER || shell->jobs[job_id] == NULL) {
        return -1;
    }

    for (p_process = shell->jobs[job_id]->first_pr; p_process != NULL; p_process = p_process->next) {
        if ((filter == MX_FILTER_DONE && p_process->status == MX_STATUS_DONE)
        || (filter == MX_FILT_IN_PROGR && p_process->status != MX_STATUS_DONE)
        || filter == MX_FILTER_ALL) {
            count++;
        }
    }
    
    return count;
}

void mx_set_process_status(Prompt *shell, int pid, int status) {
    int i;
    Process *p_process;
    int job_id = mx_job_id_by_pid(shell, pid);

    for (i = 1; i < shell->max_number_job; i++) {
        if (shell->jobs[i] == NULL) {
            continue;
        }
        
        for (p_process = shell->jobs[i]->first_pr; p_process != NULL; p_process = p_process->next) {
            if (p_process->pid == pid) {
                p_process->status = status;
                if (status == MX_STATUS_SUSPENDED) {
                    if (shell->jobs_stack->last && shell->jobs_stack->prev_last) {
                        shell->jobs_stack->prev_last = shell->jobs_stack->last;
                    }
                    shell->jobs_stack->last = job_id;
                }
                break;
            }
        }
    }
}

