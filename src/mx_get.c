#include "ush.h"

char *usage_func_error(void) {
    mx_printerr("ush: function usage: func() { ...; }\n");
    return NULL;
}

int mx_get_char_index_reverse(const char *str, char c) {
    if (!str || !*str) return -2;
    char *str_cpy = (char *)str;
    int i = mx_strlen(str_cpy) - 1;
    while (str_cpy[i]) {
        if (str_cpy[i] == c) return i;
        i--;
    }
    return -1;
}

char *get_func_var(char *line, int indx) {
    char *tmp_line = mx_strndup(line, indx);
    char *variable = mx_strtrim(tmp_line);

    mx_strdel(&tmp_line);
    if (mx_get_char_index_quote(variable, MX_USH_TOK_DELIM, NULL) >= 0) {
        mx_strdel(&variable);
        return mx_syntax_error("(");
    }
    return variable;
}

char *get_func_value(char *line, int end) {
    int end_index = end;
    char *value = NULL;

    if (!line[end_index] || mx_strncmp(&line[end_index], " { ", 3) != 0) return usage_func_error();
    end_index += 3;
    end = mx_get_char_index_quote(&line[end_index], "}", "\"\'`$(");
    if (end <= 0) return mx_syntax_error("{");
    value = mx_strndup(&line[end_index], end);
    return value;
}

bool mx_get_functions(char *line, Prompt *shell) {
    char *var;
    char *value;
    int i = mx_get_char_index_quote(line, "(", "\"\'`$");

    if (line[i + 1] && line[i + 1] == ')') {
        if (!(var = get_func_var(line, i))) return true;

        if (!(value = get_func_value(line, i + 2))) {
            mx_strdel(&var);
            return true;
        }

        if (shell->functions) mx_set_variable(shell->functions, var, value);
        else mx_push_export(&shell->functions, var, value);

        return true;
    }
    return false;
}

void get_git(struct dirent  *dir_st, char **usr, int *flag, char *path) {
    if (strcmp(dir_st->d_name, ".git") == 0) {
        char *git_path = mx_strjoin(path, "/.git/HEAD");
        char *git = mx_file_to_str(git_path);
        char **arr = mx_strsplit(git, '/');
        int num = 0;

        if (arr != NULL) {
            while (arr[num] != NULL) num++;
            arr[num - 1][mx_strlen(arr[num - 1]) - 1] = '\0';
            if (mx_count_substr(arr[num - 1], "\n") == 0) *usr = strdup(arr[num - 1]);
            (*flag)++;
            mx_del_strarr(&arr);
        }
        free(git);
        free(git_path);
    }
}

void find_git(int *flag, char **path, char **usr) {
	DIR *dir_ptr  = opendir(*path);
    struct dirent  *dir_st;
    char *real_path = NULL, *tmp_path;

    while ((dir_st = readdir(dir_ptr)) != 0) get_git(dir_st, usr, flag, *path);
    real_path = realpath(*path, NULL);

    if (strcmp(real_path, getenv("HOME")) == 0 || mx_count_substr(real_path, "/") <= 2) (*flag)++;
    free(real_path);
    closedir(dir_ptr);
    tmp_path = strdup(*path);
    free(*path);
    *path = mx_strjoin(tmp_path, "/..");
    free(tmp_path);
}

char *mx_get_git_info(void) {
    char *usr = NULL;
    int flag = 0;
    char *path = strdup(".");
    while(!flag) find_git(&flag, &path, &usr);
    return usr;
}

void key_add_char(int *pos, char *line, int keycode, Prompt *shell) {
    if (mx_strlen(line) >= shell->line_len) {
        shell->line_len += 1024;
        line = realloc(line, shell->line_len);
    }
    for (int i = mx_strlen(line); i > *pos; i--) line[i] = line[i - 1];
    line[*pos] = keycode;
    (*pos)++;
}

void key_read_input(int *max_len, int *keycode, char *line) {
    *max_len = mx_strlen(line);
    *keycode = 0;
    read(0, keycode, 4);
}

void key_choose (int *pos, char **line, int keycode, Prompt *shell) {
    if (keycode >= 127) mx_edit_command(keycode, pos, line, shell);
    else if (keycode < 32)  mx_exec_signal(keycode, line, pos, shell);
    else key_add_char(pos, *line, keycode, shell);
}

void key_print_command(Prompt *shell, char *line, int pos, int max_len){
    for (int i = pos; i <= mx_strlen(line); i++) printf (" ");
    for (int i = 0; i <= max_len + 2; i++) printf ("\b\x1b[2K");
    printf ("\r");
    mx_print_prompt(shell);
    printf ("%s", line);
    for (int i = 0; i < mx_strlen(line) - pos; i++) printf ("%c[1D", 27);
    fflush (NULL);
}

char *mx_get_keys(Prompt *shell) {
    char *line = mx_strnew(1024);
    int keycode = 0;
    int max_len = 0;
    int pos = 0;

    while(keycode != MX_CTRL_C && keycode != MX_ENTER) {
        mx_edit_prompt(shell);
        key_read_input(&max_len, &keycode, line);
        max_len += mx_strlen(shell->prompt);
        if(shell->git) max_len += mx_strlen(shell->git) + 7;
        key_choose (&pos, &line, keycode, shell);
        if(keycode != MX_CTRL_C) key_print_command(shell, line, pos, max_len);
    }
    return line;
}

struct termios mx_disable_term(void) {
    struct termios econ;
    struct termios t_ty;

    tcgetattr (0, &t_ty);
    econ = t_ty;
    t_ty.c_lflag &= ~(ICANON|ECHO|ISIG|BRKINT|ICRNL|INPCK|ISTRIP|IXON|OPOST|IEXTEN);
    t_ty.c_cflag |= (CS8);
    t_ty.c_cc[VMIN] = 1;
    t_ty.c_cc[VTIME] = 0;
    tcsetattr (0, TCSAFLUSH, &t_ty);
    return econ;
}

void line_jynx_maze(Prompt *shell, char *line) {
    if (shell->history_count == shell->history_size) {
        shell->history_size += 1000;
        shell->history = (char **)realloc(shell->history, shell->history_size);
    }
    
    if (strcmp(line, "") != 0) {
        shell->history[shell->history_count] = strdup(line);
        shell->history_count++;
    }
}

char *mx_get_line(Prompt *shell) {
    char *line;
    struct termios econ;
    int out = dup(1);
    int t_ty = open("/dev/t_ty", O_WRONLY);

    dup2(t_ty, 1);
    mx_edit_prompt(shell);
    econ = mx_disable_term();
    shell->line_len = 1024;
    mx_print_prompt(shell);
    line = mx_get_keys(shell);
    line_jynx_maze(shell, line);
    shell->history_index = shell->history_count;
    printf("\n");
    tcsetattr (0, TCSAFLUSH, &econ);
    dup2(out, 1);
    close(out);
    close(t_ty);
    return line;
}

void delete_index(char **arr, int i) {
    free(arr[i]);
    arr[i] = strdup("");

    for (int j = i; j >= 0; j --) {
        if (strcmp(arr[j], "") != 0) {
            free(arr[j]);
            arr[j] = strdup("");
            break;
        }
    }
}

char **get_arr(char *dir_str) {
    char **arr_str = NULL;
    int i = 0;

    arr_str = mx_strsplit(dir_str, '/');
    while (arr_str[i] != NULL) {
        if (strcmp(arr_str[i], ".") == 0) {
            free(arr_str[i]);
            arr_str[i] = strdup("");
        }
        if (strcmp(arr_str[i], "..") == 0)
            delete_index (arr_str, i);
        i++;
    }
    return arr_str;
}

char *fill_dir(char **arr_str) {
    int i = 0;
    char *dir = NULL;

    while (arr_str[i] != NULL) {
        if (strcmp(arr_str[i], "") != 0) {
            char *temp = mx_strjoin(dir, "/");
            if (dir)
                free(dir);
            dir = mx_strjoin(temp, arr_str[i]);
            free(temp);
        }
        i++;
    }

    if (!dir) {
        dir = strdup("/");
    }

    return dir;
}

char *get_dir(char *point, char *pwd) {
    char *cur_dir = strdup(pwd);
    char *dir = NULL;
    char **arr_str = NULL;

    if (point[0] == '/') {
        dir = strdup(point);
    }
    else {
        char *temp = mx_strjoin(cur_dir, "/");
        free(dir);
        dir = mx_strjoin(temp,point);
        free(temp);
    }

    arr_str = get_arr(dir);
    free(dir);
    dir = fill_dir(arr_str);
    mx_del_strarr(&arr_str);
    free(cur_dir);

    return dir;
}


char *mx_normalization (char *point, char *pwd) {
    char *str = NULL;
    str = get_dir(point, pwd);
    return str;
}

char mx_get_type(struct stat file_stat) {
    char result = '-';
    switch ((file_stat.st_mode & S_IFMT)) {
    case S_IFCHR:
        return 'c';
    case S_IFBLK:
        return 'b';
    case S_IFIFO:
        return 'p';
    case S_IFSOCK:
        return 's';
    case  S_IFLNK:
        return 'l';
    case S_IFDIR:
        return 'd';
    default:
        break;
    }
    
    return result;
}

void get_aliases_data(char *arg, char **name, char **value) {
    int indx = mx_get_char_index(arg, '=');
    int length = 0;
    *name = mx_strndup(arg, indx);
    if (arg[indx + 1] == '\"') {
        length = mx_strlen(&arg[indx + 2]);
        *value = mx_strndup(&arg[indx + 2], length - 1);
    }
}

void mx_get_aliases(char *line, Prompt *shell) {
    char **args = mx_parce_tokens(line);
    for (int i = 0; args[i]; i++) {
        char *name = NULL;
        char *value = NULL;
        get_aliases_data(args[i], &name, &value);
        if (value && name && mx_strcmp(value, name)) {
            if (shell->aliases) mx_set_variable(shell->aliases, name, value);
            else mx_push_export(&shell->aliases, name, value);
        }
    }
}
