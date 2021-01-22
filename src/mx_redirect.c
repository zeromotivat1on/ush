#include "ush.h"

static int count_sep_first_lwl(Abstract *q_ast) {
    int i = 1;

    for (; q_ast; q_ast = q_ast->next)
        if (MX_IS_SEP_FIRST_LWL(q_ast->type))
            i++;
    return i;
}
void mx_ast_clear_list(Abstract **list) {
    if (!(*list) || !list) return;

    Abstract *tmtmp_exp = *list;
    Abstract *tmp = NULL;

    while (tmtmp_exp) {
        if (tmtmp_exp->token) free(tmtmp_exp->token);
        if (tmtmp_exp->args) mx_del_strarr(&tmtmp_exp->args);
        if (tmtmp_exp->left) mx_ast_clear_list(&tmtmp_exp->left);

        tmp = tmtmp_exp->next;
        free(tmtmp_exp);
        tmtmp_exp = tmp;
    }
    *list = NULL;
}

void mx_ast_clear_all(Abstract ***list) {
    Abstract **tmtmp_exp = *list;

    for (int i = 0; tmtmp_exp[i]; i++) mx_ast_clear_list(&tmtmp_exp[i]);

    free(tmtmp_exp);
    tmtmp_exp = NULL;
}
Abstract *push_redirections(Abstract **q_ast_arr, Abstract **ast) {
    int temp_type;
    Abstract *c = (*q_ast_arr)->next;

    temp_type = (*q_ast_arr)->type;
    for (; c && MX_IS_REDIRECTION(temp_type); c = c->next, (*q_ast_arr) = (*q_ast_arr)->next) {
        mx_ast_push_back_redirection(ast, c->token, temp_type);
        temp_type = c->type;
    }

    for (c = *ast; c->next; c = c->next);
    c->type = temp_type;

    return *q_ast_arr;
}

Abstract **mx_ast_parse(Abstract *parsed_line) {
    Abstract *q = parsed_line;
    int k = count_sep_first_lwl(q);
    int i = 0;
    Abstract **ast = (Abstract **)malloc((k + 1) * sizeof(Abstract *));

    ast[i] = NULL;
    for (; q; q = q->next) {
        mx_ast_push_back(&ast[i], q->token, q->type);
        if (MX_IS_REDIRECTION(q->type)) {
            q = push_redirections(&q, &ast[i]);
        }
        if (MX_IS_SEP_FIRST_LWL(q->type) || q->type == NUL) {
            ast[++i] = NULL;
        }
    }
    return ast;
}


Abstract *ast_create_node(char *arg, int type) {
    Abstract *ast_q;

    if (!arg) {
        return NULL;
    }

    ast_q = (Abstract *)malloc(sizeof(Abstract));
    if (!ast_q) {
        return NULL;
    }

    ast_q->args = NULL;
    ast_q->token = strdup(arg);
    ast_q->type = type;
    ast_q->next = NULL;
    ast_q->left = NULL;

    return ast_q;
}

void mx_ast_push_back(Abstract **ast_head, char *arg, int type) {
    Abstract *ast_q;
    Abstract *ast_p;

    if (!ast_head || !arg) {
        return;
    }

    ast_q = ast_create_node(arg, type);
    if (!ast_q) {
        return;
    }

    ast_p = *ast_head;
    if (*ast_head == NULL) {
        *ast_head = ast_q;
        return;
    }
    else {
        while (ast_p->next != NULL)
            ast_p = ast_p->next;
        ast_p->next = ast_q;
    }
}

void mx_ast_push_back_redirection(Abstract **ast_head, char *arg, int type) {
    Abstract *ast_p;

    if (!ast_head || !arg) {
        return;
    }

    ast_p = *ast_head;
    if (*ast_head == NULL) {
        mx_printerr("u$h: trying to connect redirection to empry process.\n");
        return;
    }
    else {
        while (ast_p->next != NULL) {
            ast_p = ast_p->next;
        }
        mx_ast_push_back(&ast_p->left, arg, type);
    }
}

Redirection *redir_create_node(char *path, int type) {
    Redirection *redir_q;

    if (!path) {
        return NULL;
    }

    redir_q = (Redirection *)malloc(sizeof(Redirection));
    if (!redir_q) {
        return NULL;
    }

    redir_q->input_path = NULL;
    redir_q->output_path = NULL;

    if (MX_IS_REDIR_INP(type)) {
        redir_q->input_path = mx_strdup(path);
    }
    else if (MX_IS_REDIR_OUTP(type)) {
        redir_q->output_path = mx_strdup(path);
    }
    else {
        return NULL;
    }

    redir_q->redir_delim = type;
    redir_q->next = NULL;
    
    return (redir_q);
}

void mx_redir_push_back(Redirection **redir_head, char *path, int type) {
    Redirection *redir_q;
    Redirection *redir_p;

    if (!redir_head || !path) {
        return;
    }

    redir_q = redir_create_node(path, type);
    if (!redir_q) {
        return;
    }

    redir_p = *redir_head;
    if (*redir_head == NULL) {
        *redir_head = redir_q;
        return;
    }
    else {
        while (redir_p->next != NULL) {
            redir_p = redir_p->next;
        }
        redir_p->next = redir_q;
    }
}

void mx_redir_clear_list(Redirection **list) {
    Redirection *redir_q = *list;
    Redirection *redir_temp = NULL;

    if (!(*list) || !list) {
        return;
    }

    while (redir_q) {
        mx_strdel(&redir_q->input_path);
        mx_strdel(&redir_q->output_path);
        redir_temp = redir_q->next;
        free(redir_q);
        redir_q = redir_temp;
    }

    *list = NULL;
}
