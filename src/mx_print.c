#include "ush.h"

void mx_print_color(char *macros, char *str) {
    mx_printstr(macros);
    mx_printstr(str);
    mx_printstr(RESET);
}

void mx_print_strarr_in_line(char **result, const char *delim) {
    if (!result || !delim) {
        return;
    }

    if (result[0] == NULL) {
        mx_printstr("NULL\n");
    }

    for (int i = 0; result[i]; i++) {
        mx_printstr(result[i]);
        mx_printstr(delim);
        if (result[i + 1] == NULL){
            mx_printstr("NULL\n");
        }
    }
}

bool mx_parse_error(char *str, int k) {
    mx_printerr("ush: parse error near `");
    write(2, str, k);
    mx_printerr("\'\n");

    return true;
}

bool mx_unmached_error(char c) {
    if (c == '\"' || c == '\'') {
        mx_printerr("Odd number of quotes.\n");
        return true;
    }
    else {
        mx_printerr("Unmatched ");
        write(2, &c, 1);
        mx_printerr(".\n");
        return true;
    }
}

char *mx_syntax_error(char *str) {
    mx_printerr("ush: syntax error near unexpected token `");
    mx_printerr(str);
    mx_printerr("'\n");

    return NULL;
}

static char *get_delim_from_type(int t) {
    if (t == SEP) {
        return mx_strdup(";");
    }
    else if (t == FON) {
        return mx_strdup("&");
    }
    else if (t == R_OUTPUT) {
        return mx_strdup(">");
    }
    else if (t == OR) {
        return mx_strdup("||");
    }
    else if (t == AND) {
        return mx_strdup("&&");
    }
    else if (t == PIPE) {
        return mx_strdup("|");
    }
    else if (t == R_INPUT) {
        return mx_strdup("<");
    }
    else if (t == R_OUTPUT_DBL) {
        return mx_strdup(">>");
    }
    else if (t == R_INPUT_DBL) {
        return mx_strdup("<<");
    }

    return NULL;
}

Abstract *mx_parse_error_ush(int t, Abstract *ast_res, char *str) {
    char *delim;

    if (t != SEP) {                      
        delim = get_delim_from_type(t);
        mx_printerr("ush: parse error near `");
        write(2, delim, mx_strlen(delim));
        mx_strdel(&delim);
        mx_printerr("\'\n");
    }

    mx_ast_clear_list(&ast_res);
    mx_strdel(&str);

    return NULL;
}

static void print_all(char *command, char *error, char arg) {
    mx_printerr("ush: ");
    mx_printerr(command);
    mx_printerr(": -");
    mx_printerr(&arg);
    mx_printerr(": invalid option\n");
    mx_printerr(command);
    mx_printerr(": usage: ");
    mx_printerr(command);
    mx_printerr(error);
    mx_printerr(" \n");
}

int mx_count_options(char **args, char *options, char *command, char *error) {
    int n_options = 0;

    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] != '-' || strcmp(args[i], "-") == 0) break;
        if (strcmp(args[i], "--") == 0) {
            n_options++;
            break;
        }
        for (int j = 1; j < mx_strlen(args[i]); j++) {
            if(mx_get_char_index(options,args[i][j]) < 0) {
                print_all(command, error, args[i][j]);
                return -1;
            }
        }
        n_options++;
    }
    return n_options;
}

static void print_left(Abstract *q) {
    for (Abstract *tmp_left = q->left; tmp_left; tmp_left = tmp_left->next) {
        mx_printstr("redir == ");

        if (tmp_left->type == R_INPUT) mx_printstr("< ");
        else if (tmp_left->type == R_INPUT_DBL) mx_printstr("<< ");
        else if (tmp_left->type == R_OUTPUT) mx_printstr("> ");
        else if (tmp_left->type == R_OUTPUT_DBL) mx_printstr(">> ");
        
        if (tmp_left->args)
            mx_print_strarr_in_line(tmp_left->args, " ");
        else if (tmp_left->token) {
            mx_printstr(q->token);
            mx_printstr("\n");
        }
    }
}

static void print_list(Abstract *parsed_line) {
    for (Abstract *q = parsed_line; q; q = q->next) {
        mx_printstr("proc  == ");
        if (q->args)
            mx_print_strarr_in_line(q->args, " ");
        else if (q->token) {
            mx_printstr(q->token);
            mx_printstr("\n");
        }
        if (q->left)
            print_left(q);
        mx_printstr("delim == ");
        mx_printint(q->type);
        mx_printstr("\n");
    }
}

void mx_ast_print(Abstract **ast) {
    char *tmp = NULL;
    for (int i = 0; ast[i]; i++) {
        mx_print_color(MX_YEL, "job-");
        tmp = mx_itoa(i + 1);
        mx_print_color(MX_YEL, tmp);
        mx_strdel(&tmp);
        mx_printstr("\n");
        print_list(ast[i]);
    }
    mx_print_color(MX_YEL, "-----\n");
}
