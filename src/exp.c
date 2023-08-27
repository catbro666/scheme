#include "exp.h"

#include "err.h"
#include "symbol.h"
#include "number.h"
#include "pair.h"
#include "vector.h"
#include "xform.h"

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

scm_object *sym_let_syntax = NULL;
scm_object *sym_letrec_syntax = NULL;
scm_object *sym_define_syntax = NULL;
scm_object *sym_syntax_rules = NULL;
scm_object *sym_ellipsis = NULL;
scm_object *sym_underscore = NULL;

static void bad_syntax(scm_object *exp, const char *kw) {
    scm_error_object(exp, "%s: bad syntax in: ", kw);
}

static void set_error(scm_object *exp, const char *err) {
    scm_error_object(exp, "set!: %s in: ", err);
}

static void define_error(scm_object *exp, const char *err) {
    scm_error_object(exp, "define: %s in: ", err);
}

static void lambda_error(scm_object *exp, const char *err) {
    scm_error_object(exp, "lambda: %s in: ", err);
}

static void begin_error(scm_object *exp, const char *err) {
    scm_error_object(exp, "begin: %s in: ", err);
}

static void app_error(scm_object *exp, const char *err) {
    scm_error_object(exp, "#%%app: %s in: ", err);
}

static void syntax_rules_error(scm_object *exp, const char *err) {
    scm_error_object(exp, "syntax-rules: %s in: ", err);
}

int scm_exp_is_self_evaluation(scm_object *exp) {
    scm_type type = exp->type;
    return type >= scm_type_void && type <= scm_type_string;
}

/* (quote <text>) */
void scm_exp_check_quote(scm_object *exp) {
    if (scm_list_length(exp) != 2)
        bad_syntax(exp, "quote");
}

scm_object *scm_exp_get_quote_text(scm_object *exp) {
    return scm_cadr(exp);
}

/* (set! <var> <value>) */
void scm_exp_check_assignment(scm_object *exp) {
    if (scm_list_length(exp) != 3)
        set_error(exp, "bad syntax");

    scm_object *o = scm_cadr(exp);
    if (!IS_IDENTIFIER(o))
        set_error(o, "not an identifier");
}

scm_object *scm_exp_get_assignment_var(scm_object *exp) {
    return scm_cadr(exp);
}

scm_object *scm_exp_get_assignment_val(scm_object *exp) {
    return scm_caddr(exp);
}

/* (lambda <formals> <body>) */
void scm_exp_check_lambda(scm_object *exp) {
    int len = scm_list_length(exp);
    if (len < 2)
        lambda_error(exp, "bad syntax");

    scm_object *e2 = scm_cadr(exp);
    scm_object *o;
    while (e2->type == scm_type_pair) {
       o = scm_car(e2);
       if (!IS_IDENTIFIER(o))
           lambda_error(o, "not an identifier");
       e2 = scm_cdr(e2);
    }
    if (e2 != scm_null && !IS_IDENTIFIER(e2))
        lambda_error(e2, "not an identifier");
    if (len == 2)
        lambda_error(exp, "no expression in body");
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
void scm_exp_check_definition(scm_object *exp) {
    int len = scm_list_length(exp);
    if (len < 2)
        define_error(exp, "bad syntax");

    scm_object *e2 = scm_cadr(exp);
    if (!IS_IDENTIFIER(e2) &&
        e2->type != scm_type_pair)
        define_error(exp, "bad syntax");
    scm_object *o;
    if (e2->type == scm_type_pair) {
        do {
            o = scm_car(e2);
            if (!IS_IDENTIFIER(o))
                define_error(o, "not an identifier");
            e2 = scm_cdr(e2);
        } while (e2->type == scm_type_pair);
        if (e2 != scm_null && !IS_IDENTIFIER(e2))
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
}

scm_object *scm_exp_get_definition_var(scm_object *exp) {
    scm_object *e2 = scm_cadr(exp);
    if (IS_IDENTIFIER(e2)) {
        return e2;
    }
    else {
        return scm_car(e2);
    }
}

scm_object *scm_exp_get_definition_val(scm_object *exp) {
    scm_object *e2 = scm_list_ref(exp, 1);
    if (IS_IDENTIFIER(e2)) {
        return scm_list_ref(exp, 2);
    }
    else {
        return make_lambda(scm_cdr(e2), scm_cddr(exp));
    }
}

/* (if <test> <consequent> <alternate>) */
void scm_exp_check_if(scm_object *exp) {
    int len = scm_list_length(exp);
    if (len != 3 && len != 4)
        bad_syntax(exp, "if");
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
void scm_exp_check_begin(scm_object *exp) {
    int len = scm_list_length(exp);
    if (len < 0)
        begin_error(exp, "bad syntax");
    if (len == 1)
        begin_error(exp, "no expressions in begin");
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
void scm_exp_check_qq(scm_object *exp) {
    if (scm_list_length(exp) != 2)
        bad_syntax(exp, "quasiquote");
}

scm_object *scm_exp_get_qq_exp(scm_object *exp) {
    return scm_cadr(exp);
}

/* (unquote <exp>) */
void scm_exp_check_unquote(scm_object *exp) {
    if (scm_list_length(exp) != 2)
        bad_syntax(exp, "unquote");
}

scm_object *scm_exp_get_unquote_exp(scm_object *exp) {
    return scm_cadr(exp);
}

/* (unquote-splicing <exp>) */
void scm_exp_check_unquote_splicing(scm_object *exp) {
    if (scm_list_length(exp) != 2)
        bad_syntax(exp, "unquote-splicing");
}

scm_object *scm_exp_get_unquote_splicing_exp(scm_object *exp) {
    return scm_cadr(exp);
}

int scm_exp_is_syntax_or_application(scm_object *exp) {
    return IS_PAIR(exp);
}

void scm_exp_check_application(scm_object *exp) {
    if (scm_list_length(exp) < 0)
        app_error(exp, "bad syntax");
}

void scm_exp_check_syntax(scm_object *exp, const char *kw) {
    if (scm_list_length(exp) < 0)
        bad_syntax(exp, kw);
}

scm_object *scm_exp_get_application_operator(scm_object *exp) {
    return scm_car(exp);
}

scm_object *scm_exp_get_application_operands(scm_object *exp) {
    return scm_cdr(exp);
}

/* check syntax rules */
static void check_syntax_rules(scm_object *spec) {
    int len = scm_list_length(spec);
    if (len < 2)
        bad_syntax(spec, "syntax-rules");
    if (!scm_eq(scm_exp_get_spec_keyword(spec), sym_syntax_rules))
        syntax_rules_error(spec, "only a `syntax-rules' form is allowed");

    /* check literals */
    scm_object *literals = scm_exp_get_spec_literals(spec);
    scm_object *l = literals;
    scm_object *o;
    while (l->type == scm_type_pair) {
        o = scm_car(l);
        if (!IS_IDENTIFIER(o))
            syntax_rules_error(o, "<literals> must be a list of identifiers");
        if (scm_eq(o, sym_ellipsis))
            syntax_rules_error(o, "`...` is not allowed in <literals>");
        if (scm_eq(o, sym_underscore))
            syntax_rules_error(o, "`_` is not allowed in <literals>");
        l = scm_cdr(l);
    }
    if (l != scm_null)
        syntax_rules_error(literals, "<literals> must be a list of identifiers");

    scm_object *rules = scm_exp_get_spec_rules(spec);
    l = rules;
    while (l->type == scm_type_pair) {
        check_syntax_rule(scm_car(l), literals);
        l = scm_cdr(l);
    }
    //if (l != scm_null)
    //    syntax_rules_error(spec, "<transformer spec> must be a list");
}

static void check_macro_binding(scm_object *binding) {
    if (scm_list_length(binding) != 2)
        scm_error_object(binding, "macro-binding: must be the form of `(<keyword> <transformer spec>)` in: ");
    scm_object *kw = scm_car(binding);
    if (!IS_IDENTIFIER(kw))
        scm_error_object(kw, "macro-binding: <keyword> must be an identifier in: ");

    check_syntax_rules(scm_cadr(binding));
}

/* (let-syntax <bindings> <body>)
 * (letrec-syntax <bindings> <body>)
 * <bindings>: ((<keyword> <transformer spec>) ...)
 * <transformer spec>: (syntax-rules <literals> <syntax rule> ...)
 * <syntax rule>: (<pattern> <template>) */
void scm_exp_check_let_syntax(scm_object *exp) {
    if (scm_list_length(exp) < 3)
        bad_syntax(exp, "let-syntax");

    scm_object *bindings = scm_cadr(exp);

    while (bindings->type == scm_type_pair) {
        check_macro_binding(scm_car(bindings));
        bindings = scm_cdr(bindings);
    }
}

void scm_exp_check_letrec_syntax(scm_object *exp) {
    if (scm_list_length(exp) < 3)
        bad_syntax(exp, "letrec-syntax");

    scm_object *bindings = scm_cadr(exp);

    while (bindings->type == scm_type_pair) {
        check_macro_binding(scm_car(bindings));
        bindings = scm_cdr(bindings);
    }
}

/* (define-syntax <keyword> <transformer spec>)
 * <transformer spec>: (syntax-rules <literals> <syntax rule> ...)
 * <syntax rule>: (<pattern> <template>) */
void scm_exp_check_define_syntax(scm_object *exp) {
    check_macro_binding(scm_cdr(exp));
}


scm_object *scm_exp_get_spec_first_rule(scm_object *exp) {
    return scm_car(exp);
}

scm_object *scm_exp_get_spec_rest_rules(scm_object *exp) {
    return scm_cdr(exp);
}

int scm_exp_is_spec_empty_rules(scm_object *exp) {
    return exp == scm_null;
}

scm_object *scm_exp_get_spec_keyword(scm_object *exp) {
    return scm_car(exp);
}

scm_object *scm_exp_get_spec_literals(scm_object *exp) {
    return scm_cadr(exp);
}

scm_object *scm_exp_get_spec_rules(scm_object *exp) {
    return scm_cddr(exp);
}

scm_object *scm_exp_get_binding_keyword(scm_object *exp) {
    return scm_car(exp);
}

scm_object *scm_exp_get_binding_spec(scm_object *exp) {
    return scm_cadr(exp);
}

scm_object *scm_exp_get_syntax_bindings(scm_object *exp) {
    return scm_cadr(exp);
}

scm_object *scm_exp_get_syntax_first_binding(scm_object *bindings) {
    return scm_car(bindings);
}

scm_object *scm_exp_get_syntax_rest_bindings(scm_object *bindings) {
    return scm_cdr(bindings);
}

int scm_exp_is_syntax_empty_bindings(scm_object *bindings) {
    return bindings == scm_null;
}

scm_object *scm_exp_get_let_syntax_body(scm_object *exp) {
    return scm_cddr(exp);
}

scm_object *scm_exp_get_define_syntax_binding(scm_object *exp) {
    return scm_cdr(exp);
}


static int initialized = 0;

int scm_exp_init(void) {
    if (initialized) return 0;

    sym_quote = scm_symbol_new("quote", 5);
    sym_quasiquote = scm_symbol_new("quasiquote", 10);
    sym_unquote = scm_symbol_new("unquote", 7);
    sym_unquote_splicing = scm_symbol_new("unquote-splicing", 16);
    
    sym_lambda = scm_symbol_new("lambda", 6);
    sym_if = scm_symbol_new("if", 2);
    sym_set = scm_symbol_new("set!", 4);
    sym_begin = scm_symbol_new("begin", 5);
    sym_cond = scm_symbol_new("cond", 4);
    sym_and = scm_symbol_new("and", 3);
    sym_or = scm_symbol_new("or", 2);
    sym_case = scm_symbol_new("case", 4);
    sym_let = scm_symbol_new("let", 3);
    sym_let_astro = scm_symbol_new("let*", 4);
    sym_letrec = scm_symbol_new("letrec", 6);
    sym_do = scm_symbol_new("do", 2);
    sym_delay = scm_symbol_new("delay", 5);
    
    sym_else = scm_symbol_new("else", 4);
    sym_define = scm_symbol_new("define", 6);
    sym_cond_apply = scm_symbol_new("=>", 2);

    sym_let_syntax = scm_symbol_new("let-syntax", 10);
    sym_letrec_syntax = scm_symbol_new("letrec-syntax", 13);
    sym_define_syntax = scm_symbol_new("define-syntax", 13);
    sym_syntax_rules = scm_symbol_new("syntax-rules", 12);
    sym_ellipsis = scm_symbol_new("...", 3);
    sym_underscore = scm_symbol_new("_", 1);


    initialized = 1;
    return 0;
}
