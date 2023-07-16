#include "exp.h"

#include "err.h"
#include "symbol.h"
#include "pair.h"

#include <stdlib.h>

scm_object *sym_quote = NULL;
scm_object *sym_quasiquote = NULL;
scm_object *sym_unquote = NULL;
scm_object *sym_unquote_splicing = NULL;

scm_object *sym_lambda = NULL;
scm_object *sym_if = NULL;
scm_object *sym_set = NULL;
scm_object *sym_begin = NULL;
scm_object *sym_cond = NULL;
scm_object *sym_and = NULL;
scm_object *sym_or = NULL;
scm_object *sym_case = NULL;
scm_object *sym_let = NULL;
scm_object *sym_let_astro = NULL;
scm_object *sym_letrec = NULL;
scm_object *sym_do = NULL;
scm_object *sym_delay = NULL;

scm_object *sym_else = NULL;
scm_object *sym_define = NULL;
scm_object *sym_cond_apply = NULL;

const int keywords_num = 20;
scm_object *keywords[keywords_num];

static int is_keyword(scm_object *exp) {
    for (int i = 0; i < keywords_num; ++i) {
        if (scm_eq(exp, keywords[i]))
            return 1;
    }
    return 0;
}

static void variable_error(scm_object *exp) {
    char *str = scm_symbol_get_string(exp);
    scm_error("%s: bad syntax in: %s", str, str);
}

static void bad_syntax(scm_object *exp, char *kw) {
    scm_error_object(exp, "%s: bad syntax in: ", kw);
}

static void set_error(scm_object *exp, char *err) {
    scm_error_object(exp, "set!: %s in: ", err);
}

static void define_error(scm_object *exp, char *err) {
    scm_error_object(exp, "define: %s in: ", err);
}

static void lambda_error(scm_object *exp, char *err) {
    scm_error_object(exp, "lambda: %s in: ", err);
}

static void begin_error(scm_object *exp, char *err) {
    scm_error_object(exp, "begin: %s in: ", err);
}

static void app_error(scm_object *exp, char *err) {
    scm_error_object(exp, "#%%app: %s in: ", err);
}

int scm_exp_is_self_evaluation(scm_object *exp) {
    scm_type type = exp->type;
    return (type >= scm_type_void && type <= scm_type_string) ||
            type == scm_type_vector;    /* it's reasonable to add vector */
}

int scm_exp_is_variable(scm_object *exp) {
    if (exp->type != scm_type_identifier)
        return 0;
    else if (is_keyword(exp))
        variable_error(exp);

    return 1;
}

/* (<keyword> ...) */
static int scm_exp_prefix_keyword(scm_object *exp, scm_object *kw) {
    return exp->type == scm_type_pair &&
           scm_eq(scm_car(exp), kw);
}

/* (quote <text>) */
int scm_exp_is_quote(scm_object *exp) {
    if (!scm_exp_prefix_keyword(exp, sym_quote))
        return 0;
    else if (scm_list_length(exp) != 2)
        bad_syntax(exp, "quote");

    return 1;
}

scm_object *scm_exp_get_quote_text(scm_object *exp) {
    return scm_cadr(exp);
}

/* (set! <var> <value>) */
int scm_exp_is_assignment(scm_object *exp) {
    if (!scm_exp_prefix_keyword(exp, sym_set))
        return 0;
    if (scm_list_length(exp) != 3)
        set_error(exp, "bad syntax");

    scm_object *o = scm_cadr(exp);
    if (o->type != scm_type_identifier)
        set_error(o, "not an identifier");

    return 1;
}

scm_object *scm_exp_get_assignment_var(scm_object *exp) {
    return scm_cadr(exp);
}

scm_object *scm_exp_get_assignment_val(scm_object *exp) {
    return scm_caddr(exp);
}

/* (lambda <formals> <body>) */
int scm_exp_is_lambda(scm_object *exp) {
    if (!scm_exp_prefix_keyword(exp, sym_lambda))
        return 0;
    int len = scm_list_length(exp);
    if (len < 2)
        lambda_error(exp, "bad syntax");

    scm_object *e2 = scm_cadr(exp);
    scm_object *o;
    while (e2->type == scm_type_pair) {
       o = scm_car(e2);
       if (o->type != scm_type_identifier)
           lambda_error(o, "not an identifier");
       e2 = scm_cdr(e2);
    }
    if (e2 != scm_null && e2->type != scm_type_identifier)
        lambda_error(e2, "not an identifier");
    if (len == 2)
        lambda_error(exp, "no expression in body");

    return 1;
}

scm_object *scm_exp_get_lambda_parameters(scm_object *exp) {
    return scm_cadr(exp);
}

scm_object *scm_exp_get_lambda_body(scm_object *exp) {
    return scm_cddr(exp);
}

static scm_object *make_lambda(scm_object *params, scm_object *body) {
    return scm_cons(sym_lambda, scm_cons(params, body));
}

/* (deifne <var> <exp>)
 * (define (<var> <def formals>) <body>) */
int scm_exp_is_definition(scm_object *exp) {
    if (!scm_exp_prefix_keyword(exp, sym_define))
        return 0;
    int len = scm_list_length(exp);
    if (len < 2)
        define_error(exp, "bad syntax");

    scm_object *e2 = scm_cadr(exp);
    if (e2->type != scm_type_identifier &&
        e2->type != scm_type_pair)
        define_error(exp, "bad syntax");
    scm_object *o;
    if (e2->type == scm_type_pair) {
        do {
            o = scm_car(e2);
            if (o->type != scm_type_identifier)
                define_error(o, "not an identifier");
            e2 = scm_cdr(e2);
        } while (e2->type == scm_type_pair);
        if (e2 != scm_null && e2->type != scm_type_identifier)
            define_error(e2, "not an identifier");
        
        if (len == 2)
            define_error(exp, "no expression in body");
    }
    else { /* scm_type_identifier */
        if (len == 2)
            define_error(exp, "bad syntax (missing expression after identifier)");
        if (len > 3)
            define_error(exp, "bad syntax (multiple expression after identifier)");
    }
    return 1;
}

scm_object *scm_exp_get_definition_var(scm_object *exp) {
    scm_object *e2 = scm_cadr(exp);
    if (e2->type == scm_type_identifier) {
        return e2;
    }
    else {
        return scm_car(e2);
    }
}

scm_object *scm_exp_get_definition_val(scm_object *exp) {
    scm_object *e2 = scm_list_ref(exp, 1);
    if (e2->type == scm_type_identifier) {
        return scm_list_ref(exp, 2);
    }
    else {
        return make_lambda(scm_cdr(e2), scm_cddr(exp));
    }
}

/* (if <test> <consequent> <alternate>) */
int scm_exp_is_if(scm_object *exp) {
    if (!scm_exp_prefix_keyword(exp, sym_if))
        return 0;
    int len = scm_list_length(exp);
    if (len != 3 && len != 4)
        bad_syntax(exp, "if");

    return 1;
}

static scm_object *make_if(scm_object *test, scm_object *consequent, scm_object *alternate) {
    return scm_list(4, sym_if, test, consequent, alternate);
}

scm_object *scm_exp_get_if_test(scm_object *exp) {
    return scm_list_ref(exp, 1);
}

scm_object *scm_exp_get_if_consequent(scm_object *exp) {
    return scm_list_ref(exp, 2);
}

scm_object *scm_exp_get_if_alternate(scm_object *exp) {
    if (scm_list_length(exp) == 4)
        return scm_list_ref(exp, 3);
    else
        return scm_void;
}

/* (begin <tail sequence>) */
int scm_exp_is_begin(scm_object *exp) {
    if (!scm_exp_prefix_keyword(exp, sym_begin))
        return 0;
    int len = scm_list_length(exp);
    if (len < 0)
        begin_error(exp, "bad syntax");
    if (len == 1)
        begin_error(exp, "no expressions in begin");

    return 1;
}

scm_object *scm_exp_get_begin_sequence(scm_object *exp) {
    return scm_cdr(exp);
}

int scm_exp_is_sequence_last(scm_object *exp) {
    return scm_cdr(exp) == scm_null;
}

scm_object *scm_exp_get_sequence_first(scm_object *exp) {
    return scm_car(exp);
}

scm_object *scm_exp_get_sequence_rest(scm_object *exp) {
    return scm_cdr(exp);
}

/* (quasiquote <exp>) */
int scm_exp_is_qq(scm_object *exp) {
    if (!scm_exp_prefix_keyword(exp, sym_quasiquote))
        return 0;
    else if (scm_list_length(exp) != 2)
        bad_syntax(exp, "quasiquote");

    return 1;
}

scm_object *scm_exp_get_qq_exp(scm_object *exp) {
    return scm_cadr(exp);
}

/* (unquote <exp>) */
int scm_exp_is_unquote(scm_object *exp) {
    if (!scm_exp_prefix_keyword(exp, sym_unquote))
        return 0;
    else if (scm_list_length(exp) != 2)
        bad_syntax(exp, "unquote");

    return 1;
}

scm_object *scm_exp_get_unquote_exp(scm_object *exp) {
    return scm_cadr(exp);
}

/* (unquote-splicing <exp>) */
int scm_exp_is_unquote_splicing(scm_object *exp) {
    if (!scm_exp_prefix_keyword(exp, sym_unquote_splicing))
        return 0;
    else if (scm_list_length(exp) != 2)
        bad_syntax(exp, "unquote-splicing");

    return 1;
}

scm_object *scm_exp_get_unquote_splicing_exp(scm_object *exp) {
    return scm_cadr(exp);
}

int scm_exp_is_application(scm_object *exp) {
    if (exp == scm_null)
        app_error(exp, "missing procedure expression");
    if (exp->type != scm_type_pair)
        return 0;
    if (scm_list_length(exp) < 0)
        app_error(exp, "bad syntax");

    return 1;
}

scm_object *scm_exp_get_application_operator(scm_object *exp) {
    return scm_car(exp);
}

scm_object *scm_exp_get_application_operands(scm_object *exp) {
    return scm_cdr(exp);
}


static int initialized = 0;

int scm_exp_init(void) {
    if (initialized) return 0;
    int i = 0;

    sym_quote = scm_symbol_new("quote", 5);
    keywords[i++] = sym_quote;
    sym_quasiquote = scm_symbol_new("quasiquote", 10);
    keywords[i++] = sym_quasiquote;
    sym_unquote = scm_symbol_new("unquote", 7);
    keywords[i++] = sym_unquote;
    sym_unquote_splicing = scm_symbol_new("unquote-splicing", 16);
    keywords[i++] = sym_unquote_splicing;
    
    sym_lambda = scm_symbol_new("lambda", 6);
    keywords[i++] = sym_lambda;
    sym_if = scm_symbol_new("if", 2);
    keywords[i++] = sym_if;
    sym_set = scm_symbol_new("set!", 4);
    keywords[i++] = sym_set;
    sym_begin = scm_symbol_new("begin", 5);
    keywords[i++] = sym_begin;
    sym_cond = scm_symbol_new("cond", 4);
    keywords[i++] = sym_cond;
    sym_and = scm_symbol_new("and", 3);
    keywords[i++] = sym_and;
    sym_or = scm_symbol_new("or", 2);
    keywords[i++] = sym_or;
    sym_case = scm_symbol_new("case", 4);
    keywords[i++] = sym_case;
    sym_let = scm_symbol_new("let", 3);
    keywords[i++] = sym_let;
    sym_let_astro = scm_symbol_new("let*", 4);
    keywords[i++] = sym_let_astro;
    sym_letrec = scm_symbol_new("letrec", 6);
    keywords[i++] = sym_letrec;
    sym_do = scm_symbol_new("do", 2);
    keywords[i++] = sym_do;
    sym_delay = scm_symbol_new("delay", 5);
    keywords[i++] = sym_delay;
    
    sym_else = scm_symbol_new("else", 4);
    keywords[i++] = sym_else;
    sym_define = scm_symbol_new("define", 6);
    keywords[i++] = sym_define;
    sym_cond_apply = scm_symbol_new("=>", 2);
    keywords[i++] = sym_cond_apply;

    initialized = 1;
    return 0;
}
