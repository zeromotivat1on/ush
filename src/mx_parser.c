#include "ush.h"

void func_push(Abstract **res, char *arg, int type, Prompt *shell) {
    Export *q;
    char *tmp = mx_strdup(arg);
    char *args0 = mx_strtok(tmp, MX_USH_TOK_DELIM);
    for (q = shell->aliases; q; q = q->next) {
        if (mx_strcmp(args0, q->name) == 0 && !mx_strstr(q->value, args0)) {
            mx_strdel(&tmp);
            *res = mx_ush_parsed_line(*res, q->value, shell, type);
            return;
        }
    }
    for (q = shell->functions; q; q = q->next) {
        if (mx_strcmp(args0, q->name) == 0 && !mx_strstr(q->value, args0)) {
            mx_strdel(&tmp);
            *res = mx_ush_parsed_line(*res, q->value, shell, type);
            return;
        }
    }
    mx_strdel(&tmp);
    mx_ast_push_back(res, arg, type);
}
static int get_type(char *delim) {
    if (MX_IS_SEP(delim)) return SEP;
    else if (MX_IS_FON(delim)) return FON;
    else if (MX_IS_AND(delim)) return AND;
    else if (MX_IS_OR(delim)) return OR;
    else if (MX_IS_PIPE(delim)) return PIPE;
    else if (MX_IS_R_INPUT(delim)) return R_INPUT;
    else if (MX_IS_R_INPUT_DBL(delim)) return R_INPUT_DBL;
    else if (MX_IS_R_OUTPUT(delim)) return R_OUTPUT;
    else if (MX_IS_R_OUTPUT_DBL(delim)) return R_OUTPUT_DBL;
    return NUL;
}
static int get_delim(char *line, int *pos) {
    char *delim = NULL;
    int type = 0;

    if (line[0] && mx_isdelim(line[0], MX_PARSE_DELIM)) {
        if (line[1] && mx_isdelim(line[1], MX_PARSE_DELIM)) delim = mx_strndup(line, 2);
        else delim = mx_strndup(line, 1);
    }
    type = get_type(delim);
    if (delim)  *pos += mx_strlen(delim);
    mx_strdel(&delim);
    return type;
}

char *mx_get_token_and_delim(char *line, int *indx, int *type) {
    int pos = 0;
    char *temp = NULL;

    if ((pos = mx_get_char_index_quote(&line[pos], MX_PARSE_DELIM, MX_QUOTE)) > 0) {
        temp = mx_strndup(line, pos);
        *type = get_delim(line + pos, &pos);
        *indx += pos;
    } else if (pos == 0) {
        (*indx)++;
    } else {
        temp = mx_strdup(line);
        *type = SEP;
        *indx += mx_strlen(line);
    }
    return temp;
}

void recurcion_func_alias(Abstract **res, int old) {
    if (old) {
        Abstract *q = *res;
        for (;q->next;)
            q = q->next;
        q->type = old;
    }
}

bool isempty(char *s, char *delim) {
    if (!s || !delim)
        return true;
    for (int i = 0; s[i]; i++) {
        if (!mx_isdelim(s[i], delim))
            return false;
    }
    return true;
}

Abstract *mx_ush_parsed_line(Abstract *res, char *line1, Prompt *shell, int old) {
    char *line;
    int i = 0;
    char *tmp = NULL;
    int type = 0;

    if (mx_check_parce_errors(line1))
        return NULL;
    line = mx_strdup(line1);
    for (;line[i];) {
        if ((tmp = mx_get_token_and_delim(&line[i], &i, &type))) {
            if (!isempty(tmp, MX_USH_TOK_DELIM))
                func_push(&res, tmp, type, shell);
            else if (type != SEP)
                return mx_parse_error_ush(type, res, line);
            free(tmp);
        }
    }
    mx_strdel(&line);
    recurcion_func_alias(&res, old);
    return res;
}

Abstract **mx_ast_creation(char *line, Prompt *shell) {
    Abstract **ast = NULL;
    Abstract *parsed_line = NULL;

    if (!(parsed_line = mx_ush_parsed_line(parsed_line, line, shell, 0))) return NULL;

    if (!(ast = mx_ast_parse(parsed_line)) || !(*ast)) {
        mx_ast_clear_list(&parsed_line);
        return NULL;
    }

    mx_ast_clear_list(&parsed_line);
    return ast;
}

char *mx_ush_read_line(Prompt *shell) {
    size_t bufsize = 0;
    char *res = NULL;
    char *line = mx_strnew(1);

    if (getline(&line, &bufsize, stdin) < 0 && !isatty(0)) {
        shell->exit_code = 0;
        mx_clear_all(shell);
        exit(0);
    }
    if(line[0] != '\0'){
        res = mx_strdup(line);
        mx_strdel(&line);
    }
    return res;
}

static char **create_tokens(char **tokens_arr, int *bufsize_arr) {
    if (tokens_arr == NULL) {
        tokens_arr = malloc((*bufsize_arr) * sizeof(char*));

        if (!tokens_arr) {
            mx_printerr_red("ush: allocation error\n");
            return NULL;
        }
    }
    else {
        (*bufsize_arr) += 64;
        tokens_arr = realloc(tokens_arr, (*bufsize_arr) * sizeof(char*));

        if (!tokens_arr) {
            mx_printerr_red("ush: allocation error\n");
            return NULL;
        }
    }
    return tokens_arr;
}

char **mx_parce_tokens(char *str) {
    int bufsize = 64;
    int position = 0;
    char **tokens_arr = NULL;
    
    tokens_arr = create_tokens(tokens_arr, &bufsize);
    char *token_arr = mx_strtok(str, MX_USH_TOK_DELIM);

    while (token_arr != NULL) {
        tokens_arr[position] = token_arr;
        position++;
        if (position >= bufsize) {
            tokens_arr = create_tokens(tokens_arr, &bufsize);
        }
        token_arr = mx_strtok(NULL, MX_USH_TOK_DELIM);
    }
    tokens_arr[position] = NULL;
    return tokens_arr;
}
