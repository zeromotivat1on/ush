#ifndef USH_H
#define USH_H

#include "libmx.h"

#include <sys/acl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/param.h>

#include <pwd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <termios.h>
#include <term.h>
#include <curses.h>
#include <limits.h>
#include <signal.h>
#include <grp.h>

// Exit
#define MX_EXIT_FAILURE 1
#define MX_EXIT_SUCCESS 0

// Jobs
#define MX_JOBS_NUMBER 10000
#define MX_STATUS_RUNNING 0
#define MX_STATUS_DONE 1
#define MX_STATUS_SUSPENDED 2
#define MX_STATUS_CONTINUED 3
#define MX_STAT_TERMINATED 4
#define MX_FILTER_ALL 0
#define MX_FILTER_DONE 1
#define MX_FILT_IN_PROGR 2
#define MAX_LEN 10

// Timers
#define MX_WNOHANG          0x00000001
#define MX_WUNTRACED        0x00000002
#define MX_W_INT(w)         (*(int *)&(w))  // Convert union wait to int
#define MX_WSTAT(x)         (MX_W_INT(x) & 0177)
#define MX_WSTOPPED         0177            // _WSTATUS if process is stopped
#define MX_WSTOPSIG(x)      (MX_W_INT(x) >> 8)
#define MX_WIFCONT(x)       (MX_WSTAT(x) == MX_WSTOPPED && MX_WSTOPSIG(x) == 0x13)
#define MX_WIFSTOPP(x)      (MX_WSTAT(x) == MX_WSTOPPED && MX_WSTOPSIG(x) != 0x13)
#define MX_WIFEXITED(x)     (MX_WSTAT(x) == 0)
#define MX_WIFSIGNALED(x)   (MX_WSTAT(x) != MX_WSTOPPED && MX_WSTAT(x) != 0)
#define MX_WTERMSIG(x)      (MX_WSTAT(x))
#define MX_W_EXITCODE(ret, sig)    ((ret) << 8 | (sig))
#define MX_W_STOPCODE(sig)  ((sig) << 8 | MX_WSTOPPED)
#define MX_WEXITED          0x00000004  // [XSI] Processes which have exitted
#define MX_WCONTINUED       0x00000010  // [XSI] Any child stopped then continu
#define MX_WNOWAIT          0x00000020  // [XSI] Leave process returned waitabl
#define MX_SIG_DFL          (void (*)(int))0
#define MX_SIG_IGN          (void (*)(int))1
#define MX_SIG_HOLD         (void (*)(int))5
#define MX_SIG_ERR          ((void (*)(int))-1)

// Colors
#define MX_BLK          "\x1B[30m"
#define MX_RED          "\x1B[31m"
#define MX_GRN          "\x1B[32m"
#define MX_YEL          "\x1B[33m"
#define MX_BLU          "\x1B[34m"
#define MX_MAG          "\x1B[35m"
#define MX_CYN          "\x1B[36m"
#define MX_WHT          "\x1B[37m"
#define MX_RESET        "\x1B[0m"
#define MX_RED_B        "\x1B[1;31m"
#define MX_RESET_B      "\x1B[1;31m"
#define MX_BLK_F_RED_B  "\x1B[0;30;41m"
#define MX_BLK_F_CYAN_B "\x1B[0;30;46m"
#define MX_BLOCK        "\x1B[0;34;46m"
#define MX_CHR          "\x1B[0;34;43m"
#define MX_DIR_T        "\x1B[0;30;42m"
#define MX_DIR_X        "\033[0;30;43m"
#define MX_BOLD_MAGENTA "\x1B[1;35m"
#define MX_BOLD_CYAN    "\x1B[1;36m"
#define MX_BOLD_RED     "\x1B[[1;31m"
#define MX_BOLD_BLUE    "\x1B[1;34m"

// Keyboard keys
#define MX_INPUT_SIZE  1024
#define MX_K_LEFT      4479771  // Edit keys
#define MX_K_RIGHT     4414235
#define MX_K_HOME      4741915
#define MX_K_END       4610843
#define MX_K_UP        4283163  // History keys
#define MX_K_DOWN      4348699
#define MX_P_UP        2117425947
#define MX_P_DOWN      2117491483
#define MX_C_PROMPT    42946
#define MX_CTRL_D      4
#define MX_CTRL_C      3
#define MX_CTRL_R      18
#define MX_BACKSCAPE   127
#define MX_TAB         9
#define MX_ENTER       10

// Abstract syntax
#define MX_PARSE_DELIM      ";|&><"
#define MX_QUOTE            "\"\'`$("
#define MX_DBLQ_EXCEPTIONS  "$`\"\\!"
#define MX_USH_TOK_DELIM    " \t\r\n\a"

// Macroses for recognizing delimeters.
#define MX_IS_SEP(x)            (!mx_strcmp(x, ";"))
#define MX_IS_FON(x)            (!mx_strcmp(x, "&"))
#define MX_IS_AND(x)            (!mx_strcmp(x, "&&"))
#define MX_IS_OR(x)             (!mx_strcmp(x, "||"))
#define MX_IS_PIPE(x)           (!mx_strcmp(x, "|"))
#define MX_IS_R_INPUT(x)        (!mx_strcmp(x, "<"))
#define MX_IS_R_INPUT_DBL(x)    (!mx_strcmp(x, "<<"))
#define MX_IS_R_OUTPUT(x)       (!mx_strcmp(x, ">"))
#define MX_IS_R_OUTPUT_DBL(x)   (!mx_strcmp(x, ">>"))
#define MX_IS_SEP_FIRST_LWL(x)  (x == SEP || x == FON || x == AND || x == OR)
#define MX_IS_REDIR_INP(x)      (x == R_INPUT || x == R_INPUT_DBL)
#define MX_IS_REDIR_OUTP(x)     (x == R_OUTPUT || x == R_OUTPUT_DBL)
#define MX_IS_REDIRECTION(x)    (MX_IS_REDIR_INP(x) || MX_IS_REDIR_OUTP(x))

// Operators
enum _type {
    SEP,            // ;
    FON,            // &
    AND,            // &&
    OR,             // ||
    PIPE,           // |
    R_INPUT,        // <
    R_INPUT_DBL,    // <<
    R_OUTPUT,       // >
    R_OUTPUT_DBL,   // >>
    NUL
};

// Abstract Syntax
typedef struct _abst {
    char    **args;
    char    *token;
    int     type;
    struct  _abst *next;
    struct  _abst *left;
}              Abstract;

// Redirections
typedef struct _redirect {
    int     mypipe_redir[2];
    char    *input_path;    // < <<
    char    *output_path;   // > >>
    int     redir_delim;    // <, <<, >, >> from _type
    struct  _redirect *next;
}              Redirection;

typedef struct s_jobs {
    int l;
    int r;
    int s;
}              t_jobs;

typedef struct cd_s {
    int s;
    int L;
    int P;
}              cd_t;

typedef struct pwd_s {
    int L;
    int P;
}              pwd_t;

typedef struct echo_s {
    int n;
    int e;
    int E;
}              echo_t;

typedef struct which_s {
    int s;
    int a;
}              which_t;

typedef struct env_s {
    int i;
    int u;
    int P;
}              env_t;

typedef struct  _export {
    char *name;
    char *value;
    struct _export *next;
}               Export;

typedef struct  _stack {
    int         size;       // Size = MX_JOBS_NUMBER
    int*        stack;
    int         top;        // Index of last add job
    int         last;       // Current job gor fg
    int         prev_last;
}              Stack;

typedef struct _env_builtin  {
    env_t       env_options;
    int         n_options;
    int         n_variables;
    int         n_args;
    Export    *env_list;
    Export    *env_params;
    char        *path;
}              BuiltIn;

typedef struct _process {
    char    *fullpath;      // For execve
    char    **argv;         // Gets in create_job.c
    char    *command;
    char    *arg_command;
    char    *input_path;    // < <<
    char    *output_path;   // > >>
    int     redir_delim;    // <, <<, >, >> from _type
    Redirection *redirect;  // New
    int     c_input;        // Count_redir_input
    int     c_output;       // Count_redir_output
    int     *r_infile;
    int     *r_outfile;
    pid_t   pid;
    int     exit_code;
    char    *path;
    char    **env;
    int     status;         // Status RUNNING DONE SUSPENDED CONTINUED TERMINATED
    int     foregrd;
    int     pipe;           // Gets in create_job.c
    int     delim;          // Gets in create_job.c (first - | || &&) (end - ; &)
    int     type;           // COMMAND_BUILTIN = index in m_s->builtin_list; default = 0
    struct _process *next;  // Next process in pipeline
    pid_t   pgid;
    int     infile;
    int     outfile;
    int     errfile;
}             Process;

// Pipeline of processes
typedef struct _job {
    int     job_id;            // Number in jobs control
    int     job_type;          // 0 if normal, or enum &&, || of previos job
    char    *command;          // Command line, used for messages
    Process *first_pr;         // List of processes in this job
    pid_t   pgid;              // Process group ID
    char    *path;
    char    **env;
    int     flag;
    int     exit_code;
    int     foregrd;            // Foregrd = 1 or background execution = 0
    struct termios tmodes;      // Saved terminal modes
    int     infile;
    int     outfile;
    int     errfile;
    int     stdin;              // Standard i/o channels
    int     stdout;             // Standard i/o channels
    int     stderr;             // Standard i/o channels
    struct _job *next;          // Next job separated by ";" "&&" "||"
}             Job;

typedef struct _prompt {
    int     argc;
    char    **argv;               // Check usage, because the same in process
    char    **envp;               // Not used
    int     exit_code;            // Return if exit
    Job   *jobs[MX_JOBS_NUMBER];  // Arr jobs
    Stack *jobs_stack;
    int     max_number_job;    // Number of added jobs + 1
    char    **builtin_list;    // Builtin functions
    int     exit_flag;         // Defaults 0, check if you have suspended jobs
    char    **history;
    int     history_count;
    int     history_index;
    int     history_size;
    struct termios tmodes;
    struct termios t_original;
    struct termios t_custom;
    bool    custom_terminal;
    pid_t   shell_pgid;
    char    *pwd;
    char    *prompt;
    char    *git;
    int     line_len;
    int     prompt_status;
    Export *exported;
    Export *variables;
    Export *functions;
    Export *aliases;
    int     redir;
    char    *kernal;
}             Prompt;

// mx_do_bg.c
int mx_check_args(Prompt *, Process *);  // Used in fg and bg
int mx_bg_get_job_id(Prompt *, Process *);
void mx_err_j(char *, char *, char *, char *);

// mx_do_cd.c
int cd_count_args(char **, int );
void cd_fill_options(int , cd_t *, char **);
void mx_change_dir(char *, cd_t , Prompt *, int *);
char *chpwd(char **, int , Prompt *);
char *replace_sub(char *, char *, char *);
char *mx_go_somewhere(Process *, int );
char *mx_go_back(void);
char *mx_go_home(void);

// mx_do_chdir.c
int mx_chdir(Prompt *, Process *);

// mx_do_echo.c
void mx_escape_seq(Process *, int , echo_t );

// mx_do_env.c
int mx_count_env_options(char **, BuiltIn *);
int mx_add_option(char **, int *, int *, BuiltIn *);
void mx_set_data(BuiltIn *, char *[]);
void mx_launch_command( Process *, BuiltIn *, int *);
void mx_env_err(int *, int *, char );
void mx_print_env_error(char , char *);

// mx_do_exit.c
int mx_true(Prompt *, Process *);
int mx_false(Prompt *, Process *);
void mx_clear_all(Prompt *);
void mx_clear_export(Export *);
void mx_clear_list(t_list **);

// mx_do_export.c
void mx_push_export(Export **, void *, void *);
void mx_export_or_error(char *, Export *, Export *, int *);
void mx_clear_data(char *, char *);

// mx_do_fg.c

// mx_do_kil.c

// mx_do_pwd.c

// mx_do_unset.c

// mx_do_which.c
void mx_get_command_info(Prompt *, char *, int *, which_t );

// mx_error.c
void mx_sig_h(int signal);
void mx_print_error(char *, char *);
void mx_printerr_red(char *);
bool mx_check_parce_errors(char *);

// mx_get.c
int mx_get_char_index_reverse(const char *, char );
void mx_get_aliases(char *, Prompt *);
char mx_get_type(struct stat);
char *mx_normalization (char *, char *);
char *mx_get_git_info(void);
char *mx_get_keys(Prompt *);
char *mx_get_line(Prompt *);
bool mx_get_functions(char *, Prompt *);

// mx_init_prompt.c
Prompt *init_ush(int , char **);
int mx_builtin_commands_idex(Prompt *, char *);
int mx_get_proc_count(Prompt *, int , int );
int mx_launch_builtin(Prompt *, Process *, int );
int mx_get_flag(char **);
void mx_set_shell_grp(Prompt *);
void mx_termios_save(Prompt *);
void mx_termios_restore(Prompt *);
void mx_pgid(Prompt *, int , int );
void mx_set_process_status(Prompt *, int , int );
void mx_print_prompt(Prompt *);
void mx_edit_prompt(Prompt *);
void mx_edit_command(int , int *, char **, Prompt *);
void mx_exec_signal(int , char **, int *, Prompt *);
void mx_check_exit(Prompt *, Process *);
char *mx_get_shlvl(void);
char *mx_run_sub_shell(char *, Prompt *);
char *mx_subs_output(char **);
char **mx_filters(char *, Prompt *);

// mx_jobs.c
Job *mx_create_job(Prompt *, Abstract *);
int mx_jobs(Prompt *, Process *);
int mx_get_next_job_id(Prompt *);
int mx_insert_job(Prompt *, Job *);
int mx_set_job_status(Prompt *, int , int );
int mx_get_job_status(Prompt *, int , int );
int mx_g_find_job(Prompt *, char *);
int mx_job_is_running(Prompt *, int );
int mx_job_id_by_pid(Prompt *, int );
int mx_get_pgid_by_job_id(Prompt *, int );
int mx_job_completed(Prompt *, int );
int mx_wait_job(Prompt *, int );	// Waitpid  in  group
void mx_launch_job(Prompt *, Job *);
void mx_remove_job(Prompt *, int );
void mx_remove_job_from_panel(Prompt *, int );
void mx_set_last_job(Prompt *);
void mx_init_jobs_stack(Prompt *);
void mx_push_to_stack (Prompt *, int );
void mx_pop_from_stack(Prompt * , int );
void mx_print_stack (Prompt *);
void mx_print_pid_process_in_job(Prompt *, int );	// If foreg execution
void mx_print_job_status(Prompt *, int , int );
void mx_check_jobs(Prompt *);            // Waitpid any 
void mx_destroy_jobs(Prompt *, int );    // Free  memory

// mx_loop.c
void mx_ush_loop(Prompt *);

// mx_parser.c
Abstract **mx_ast_creation(char *, Prompt *);
Abstract *mx_ush_parsed_line(Abstract *, char *, Prompt *, int);
char *mx_get_token_and_delim(char *, int *, int *);
char *mx_ush_read_line(Prompt *);
char **mx_parce_tokens(char *);

// mx_print.c
Abstract *mx_parse_error_ush(int , Abstract *, char *);
int mx_count_options(char **, char *, char *, char *);
void mx_ast_print(Abstract **ast);
void mx_print_strarr_in_line(char **, const char *);
void mx_print_color(char *, char *);
char *mx_syntax_error(char *);
bool mx_unmached_error(char );
bool mx_parse_error(char *, int );

// mx_process.c
int mx_launch_process(Prompt *, Process *, int);
int mx_launch_bin(Process *, char *, char **);
void mx_push_process_back(Process **, Prompt *, Abstract *);
void mx_clear_process(Process *);
void mx_dup_fd(Process *);
void mx_dup_close(int , int );
void mx_dup2_fd(int *, int *);
void mx_print_fd(Process  *);

// mx_redirect.c
Abstract **mx_ast_parse(Abstract *);
void mx_ast_push_back(Abstract **, char *, int );
void mx_ast_push_back_redirection(Abstract **, char *, int);
void mx_ast_clear_list(Abstract **);
void mx_ast_clear_all(Abstract ***);
void mx_redir_push_back(Redirection **, char *, int);
void mx_redir_clear_list(Redirection **);

// mx_set.c
Export *mx_set_variables();
Export *mx_set_export();
int mx_set(Prompt *, Process *);
int mx_set_redirections(Prompt *, Job *, Process *);
int mx_alias(Prompt *, Process *);  
int mx_declare(Prompt *, Process *);
int mx_red_in(Job *, Process *, char *, int j);
int mx_set_parametr(char **,  Prompt *);
void mx_count_redir(Process *);
void mx_set_r_infile(Prompt *, Job  *, Process *);
void mx_set_r_outfile(Prompt *, Job *, Process *);
void mx_set_variable(Export *, char *, char *);
void mx_export_value(Export *, char *, char *);

// mx_str_funcs.c
int mx_get_char_index_quote(char *, char *, char *);
int mx_strlen_arr(char **);
void mx_strtrim_quote(char **);
void mx_print_args_in_line(char **, const char *);
char *mx_strdup_from(char *, int index);
char *mx_strjoin_free(char *, char const *);
char *mx_strtok (char *, const char *);
char **mx_strdup_arr(char **);

// mx_substring.c
char *mx_subst_tilde(char *, Export *);
char *mx_add_login(char *, char *);
char *mx_substr_dollar(char *, Export *);
char *mx_sub_str_command(char *, Prompt *);

// Builtin commands
int mx_env(Prompt *, Process *);	// mx_do_env.c
int mx_echo(Prompt *, Process *);   // mx_do_echo.c
int mx_fg(Prompt *, Process *);		// mx_do_fg.c
int mx_bg(Prompt *, Process *);		// mx_do_bg.c
int mx_cd(Prompt *, Process *);		// mx_do_cd.c
int mx_pwd(Prompt *, Process *);	// mx_do_pwd.c
int mx_export(Prompt *, Process *);	// mx_do_export.c
int mx_unset(Prompt *, Process *);	// mx_do_unset.c
int mx_which(Prompt *, Process *);	// mx_do_which.c
int mx_exit(Prompt *, Process *); 	// mx_do_exit.c
int mx_kill(Prompt *, Process *); 	// mx_do_kill.c

#endif
