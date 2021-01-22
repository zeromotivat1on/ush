#include "ush.h"

int options_count(char **args) {
    int n_options = 0;

    for (int i = 1; args[i]; i++) {
        n_options++;
    }

    return n_options;
}

int mx_set(Prompt *shell, Process *p) {
    int n_options = options_count(p->argv);
    int exit_code = shell->exit_code;

    if (n_options == 0) {
        for (Export *q = shell->variables; q; q = q->next) {
            mx_printstr(q->name);
            mx_printstr("=");
            mx_printstr(q->value);
            mx_printstr("\n");
        }
    }

    return exit_code;
}

int mx_alias(Prompt *shell, Process *p) {
    int n_options = options_count(p->argv);
    int exit_code = shell->exit_code;

    if (n_options == 0) {
        for (Export *q = shell->aliases; q; q = q->next) {
            mx_printstr(q->name);
            mx_printstr("='");
            mx_printstr(q->value);
            mx_printstr("'\n");
        }
    }
    
    return exit_code;
}

int mx_declare(Prompt *shell, Process *p) {
    int n_options = options_count(p->argv);
    int exit_code = shell->exit_code;

    if (n_options == 1 && mx_strcmp(p->argv[1], "-f") == 0) {
        for (Export *q = shell->functions; q; q = q->next) {
            mx_printstr(q->name);
            mx_printstr(" () {\n\t");
            mx_printstr(q->value);
            mx_printstr("\n}\n");
        }
    }

    return exit_code;
}

void mx_set_variable(Export *export, char *name, char *value) {
    Export *head_export = export;
    int flag = 0;

    while (head_export != NULL) {
        if (strcmp(head_export->name, name) == 0) {
            flag++;
            free(head_export->value);
            head_export->value = strdup(value);
            break;
        }
        head_export = head_export->next;
    }
    if (!flag) {
        mx_push_export(&export, name, value);
    } 
}

void mx_set_r_infile(Prompt *shell, Job *job, Process *p_process) {
    Redirection *redir;
    int j = 0;

    p_process->r_infile = (int *) realloc(p_process->r_infile, sizeof(int) * (p_process->c_input));
    p_process->r_infile[0] = job->infile;

    if (p_process->redirect) {
        for (redir = p_process->redirect; redir; redir = redir->next) {
            if (redir->input_path) {
                if (redir->redir_delim == R_INPUT) {
                    shell->redir = mx_red_in(job, p_process, redir->input_path, j);
                }
                j++;
            }
        }
        job->infile = p_process->r_infile[0];
    }
}


int mx_red_in(Job *job, Process *p_process, char *input_path, int j) {
    int status_redir = 0;
    int fd;

    if ((fd = open(input_path, O_RDONLY, 0666)) < 0) {
        mx_printerr("ush :");
        perror(input_path);
        job->exit_code = 1;
        status_redir = 1;

        return status_redir;
    }
    
    p_process->r_infile[j] = fd;
    return status_redir;
}

int set_flag(int redir_delim) {
    int flags = 0;
    
    if (redir_delim == R_OUTPUT_DBL) {
        flags = O_WRONLY | O_CREAT;
    }

    if (redir_delim == R_OUTPUT) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    }
    
    return flags;
}

int mx_set_redirections(Prompt *shell, Job *job, Process *p_process) {
    mx_count_redir(p_process);
    shell->redir = 0;

    mx_set_r_infile(shell, job, p_process);
    mx_set_r_outfile(shell, job, p_process);
    p_process->errfile = job->errfile;

    if (shell->redir == 1) {
        shell->exit_code = 1;
        job->exit_code = 1;
        mx_set_variable(shell->variables, "?", "1");
    }

    return shell->redir;
}

void mx_count_redir(Process *p_process) {
    Redirection *r;

    p_process->c_input = 0;
    p_process->c_output = 0;
    for (r = p_process->redirect; r; r = r->next) {
        if (r->redir_delim == R_OUTPUT || r->redir_delim == R_OUTPUT_DBL) {
            p_process->c_output += 1;
        }

        if (r->redir_delim == R_INPUT || r->redir_delim == R_INPUT_DBL) {
            p_process->c_input += 1;
        }
        
    }

    if (p_process->c_output == 0) {
        p_process->c_output++;
    }
    if (p_process->c_input == 0) {
        p_process->c_input++;
    }
    p_process->r_outfile = (int *) realloc(p_process->r_outfile, sizeof(int) * (p_process->c_output));
}

void mx_set_r_outfile(Prompt *shell, Job *job, Process *p_process) {
    int fd;
    Redirection *redir;
    int j = 0;
    int flags = 0;

    p_process->r_outfile[0] = job->outfile;
    if (p_process->redirect) {
        for (redir = p_process->redirect; redir; redir = redir->next) {
            if (redir->output_path) {
                flags = set_flag(redir->redir_delim);

                if ((fd = open(redir->output_path, flags, 0666)) < 0) {
                    mx_printerr("ush :");
                    perror(redir->output_path);
                    shell->redir = 1;
                    continue;
                }
                
                p_process->r_outfile[j] = fd;
                lseek(p_process->r_outfile[j], 0, SEEK_END);
                j++;
            }
        }
    }
    job->outfile = p_process->r_outfile[0];
}

static char *strdup_from(char *str, int index) {
    for (int i = 0; i <= index; i++) {
        str++;
    }
    return strdup(str);
}

static void get_data (char *arg, char **name, char **value) {
    int idx = mx_get_char_index(arg,'=');

    *value = strdup_from(arg,idx);
    *name = strndup(arg,idx);
}

void mx_export_value(Export *export, char *name, char *value) {
    Export *head = export;

    while (head != NULL) {
        if (strcmp(head->name, name) == 0) {
            if (head->value) {
                free(head->value);
            }

            head->value = strdup(value);
            setenv(name, value, 1);
            break;
        }
        head = head->next;
    }
}

int mx_set_parametr(char **args, Prompt *shell) {
    char *name;
    char *value;

    for (int i = 0; args[i] != NULL; i++) {
        name = NULL;
        value = NULL;
        get_data(args[i], &name, &value);
        if (value != NULL && name != NULL) {
            mx_set_variable(shell->variables, name, value);
            mx_export_value(shell->exported, name, value);
        }

        if (value) {
            free(value);
        }
        if (name) {
            free(name);
        }
        
    }
    return 0;
}

Export *mx_set_variables() {
    extern char** environ;
    Export *variables = NULL;;

    for (size_t i = 0; environ[i] != NULL; i++) {
        int idx = mx_get_char_index(environ[i],'=');
        char *name = strndup(environ[i],idx);
        char *value = mx_strdup_from(environ[i],idx);

        mx_push_export(&variables, name, value);

        if (value) {
            free(value);
        }
        if (name) {
            free(name);
        }
        
    }

    mx_push_export(&variables, "?", "0");
    return variables;
}

Export *mx_set_export() {
    Export *export = NULL;
    extern char** environ;

    for (size_t i = 0; environ[i] != NULL; i++) {
        int idx = mx_get_char_index(environ[i],'=');
        char *name = strndup(environ[i],idx);
        char *value = mx_strdup_from(environ[i],idx);

        if(strcmp(name, "_") != 0) {
            mx_push_export(&export, name, value); 
        }

        if (value) {
            free(value); 
        }
        if (name) {
            free(name);
        }   
    }
    
    return export;
}
