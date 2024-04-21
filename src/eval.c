#include "eval.h"

#include "err.h"
#include "symbol.h"
#include "string.h"
#include "pair.h"
#include "vector.h"
#include "proc.h"
#include "env.h"
#include "exp.h"
#include "xform.h"

#include <stdlib.h>

/* -----------------core syntax -------------------------*/
typedef scm_object *(*eval_fn)(scm_object *exp, scm_object *env);

typedef struct scm_core_syntax_st {
    scm_object base;
    scm_object *kw;
    eval_fn eval;
} scm_core_syntax;

static scm_object *core_syntax_new(scm_object *kw, eval_fn eval) {
    scm_core_syntax *syntax = malloc(sizeof(scm_core_syntax));
    syntax->base.type = scm_type_core_syntax;
    syntax->kw = kw;
    syntax->eval = eval;

    return (scm_object *)syntax;
}

static void core_syntax_free(scm_object *obj) {
    free(obj);
}

static void variable_error(scm_object *exp) {
    char *str = scm_variable_get_string(exp);
    scm_error("%s: bad syntax in: %s", str, str);
}

static void unquote_splicing_error(scm_object *exp, const char *err) {
    scm_error_object(exp, "unquote-splicing: %s in: ", err);
}

static scm_object *eval_variable(scm_object *exp, scm_object *env) {
    scm_object *o = scm_env_lookup_var(env, exp);
    if (o == NULL) {
        char *str = scm_variable_get_string(exp);
        scm_error("%s: undefined;\ncannot reference an identifier before its definition", str);
    }
    return o;
}

static scm_object *eval_assignment(scm_object *exp, scm_object *env) {
    scm_exp_check_assignment(exp);
    scm_object *var = scm_exp_get_assignment_var(exp);
    scm_object *val = scm_eval(scm_exp_get_assignment_val(exp), env);
    int res = scm_env_set_var(env, var, val);
    switch (res) {
    case 1:
        scm_error_object(var, "set!: assignment disallowed;\ncan't set "
                         "variable before its definition\nvariable: ");
        break;
    case 2:
        scm_error_object(var, "set!: cannot mutate syntax identifier in: ");
        break;
    }
    return scm_void;
}

static scm_object *eval_lambda(scm_object *exp, scm_object *env) {
    return scm_compound_new(scm_exp_get_lambda_parameters(exp),
                            scm_exp_get_lambda_body(exp), env);
}

static scm_object *eval_definition(scm_object *exp, scm_object *env) {
    scm_object *var = scm_exp_get_definition_var(exp);
    scm_object *val = scm_eval(scm_exp_get_definition_val(exp), env);
    if (val->type == scm_type_compound)
        scm_compound_set_name(val, scm_symbol_get_string(var));
    scm_env_define_var(env, var, val);

    return scm_void;
}

static scm_object *eval_if(scm_object *exp, scm_object *env) {
    if (scm_false != scm_eval(scm_exp_get_if_test(exp), env))
        return scm_eval(scm_exp_get_if_consequent(exp), env);
    else
        return scm_eval(scm_exp_get_if_alternate(exp), env);
}

scm_object *scm_eval_sequence(scm_object *exp, scm_object *env) {
    if (scm_exp_is_sequence_last(exp)) {
        /* tail call */
        return scm_eval(scm_exp_get_sequence_first(exp), env);
    }
    else {
        scm_eval(scm_exp_get_sequence_first(exp), env);
        return scm_eval_sequence(scm_exp_get_sequence_rest(exp), env);
    }
}

static scm_object *eval_begin(scm_object *exp, scm_object *env) {
    return scm_eval_sequence(scm_exp_get_begin_sequence(exp), env);
}

static scm_object *unwrap_esymbols(scm_object *exp) {
    scm_object *head = scm_null;
    scm_object *pair, *tail;
    switch (exp->type) {
    case scm_type_pair:
        FOREACH_LIST(o, exp) {
            pair = scm_cons(unwrap_esymbols(o), scm_null);
            if (head == scm_null) {
                head = pair;
            }
            else {
                scm_set_cdr(tail, pair);
            }
            tail = pair;
        }
        if (exp != scm_null) {
            scm_set_cdr(tail, unwrap_esymbols(exp));
        }
        return head;
    case scm_type_vector:
        if (exp == scm_empty_vector)
            return scm_empty_vector;
        scm_object *v = scm_vector_alloc(scm_vector_length(exp));
        FOREACH_VECTOR(i, n, exp) {
            scm_vector_set(v, i, unwrap_esymbols(scm_vector_ref(exp, i)));
        }
        return v;
    case scm_type_eidentifier:
        return scm_esymbol_get_symbol(exp);
    default:
        return exp;
    }
}

static scm_object *eval_quote(scm_object *exp, scm_object *env) {
    (void)env;
    scm_exp_check_quote(exp);
    return unwrap_esymbols(scm_exp_get_quote_text(exp));
}

static scm_object *eval_unquote(scm_object *exp, scm_object *env) {
    (void)env;
    scm_error_object(exp, "unquote: not in quasiquote in: ");
    return NULL;
}

static scm_object *eval_unquote_splicing(scm_object *exp, scm_object *env) {
    (void)env;
    scm_error_object(exp, "unquote-splicing: not in quasiquote in: ");
    return NULL;
}

static scm_object *is_syntax_unquote_or_unquote_splicing(scm_object *exp, scm_object *env) {
    if (!IS_IDENTIFIER(exp))
        return NULL;
    scm_object *o = eval_variable(exp, env);
    if (o->type == scm_type_core_syntax) {
        scm_object *kw = ((scm_core_syntax*)o)->kw;
        if (scm_eq(kw, sym_unquote) || scm_eq(kw, sym_unquote_splicing))
            return kw;
    }
    return NULL;
}


/* @in_seq: indicate if the current exp is in sequence context
 * @sp_len: out parameter, indicate if the current exp is unquote-splicing
 *          returns the length of list, otherwise returns -1 */
static scm_object *eval_qq_ex(scm_object *exp, scm_object *env, int in_seq, int *sp_len) {
    scm_object *e = NULL;
    scm_object *o = NULL;
    scm_object *kw;
    *sp_len = -1;

    kw = is_syntax_unquote_or_unquote_splicing(exp, env);
    if (kw) {
        scm_error_object(exp, "%s: invalid context within quasiquote in: ",
                         scm_symbol_get_string(kw));
    }

    if (scm_exp_is_syntax_or_application(exp)) {
        kw = is_syntax_unquote_or_unquote_splicing(
            scm_exp_get_application_operator(exp), env);

        if (kw) {
            if (scm_eq(kw, sym_unquote)) {
                scm_exp_check_unquote(exp);
                return scm_eval(scm_exp_get_unquote_exp(exp), env);
            }
            else if (scm_eq(kw, sym_unquote_splicing)) {
                scm_exp_check_unquote_splicing(exp);
                if (!in_seq)
                    unquote_splicing_error(exp, "not in context of sequence");
                o = scm_eval(scm_exp_get_unquote_splicing_exp(exp), env);
                *sp_len = scm_list_length(o);
                if (*sp_len < 0)
                    unquote_splicing_error(exp, "not returning a list");

                return o;
            }
        }
    }

    if (exp->type == scm_type_vector) {
        int n = scm_vector_length(exp);
        if (n == 0)
            return exp;

        int i = 0, j = 0;
        int sn;
        scm_object *vec = scm_vector_alloc(n);
        int vec_len = n;
        while (i < n) {
            e = scm_vector_ref(exp, i++);
            o = eval_qq_ex(e, env, 1, &sn);
            if (sn < 0) {
                scm_vector_set(vec, j++, o); 
            }
            else {
                if (sn != 1) {
                    vec_len += sn - 1;
                    vec = scm_vector_realloc(vec, vec_len);
                }
                while (sn--) {
                    scm_vector_set(vec, j++, scm_car(o));
                    o = scm_cdr(o);
                }
            }
        }
        return vec;    
    }
    if (exp->type == scm_type_pair) {
        int sn;
        scm_object *head = scm_null;
        scm_object *tail = scm_null;
        scm_object *pair = NULL;
        while (exp->type == scm_type_pair) {
            kw = is_syntax_unquote_or_unquote_splicing(scm_car(exp), env);
            if (kw)
                break;

            o = eval_qq_ex(scm_car(exp), env, 1, &sn);
            /* need to copy the list here because it may come from the env
             * we only do the shallow copy here */
            if (sn == 0) {
                /* do nothing */
            }
            else {
                /* address the first element */
                if (sn < 0) {
                    pair = scm_cons(o, scm_null);
                    o = scm_null;
                }
                else {
                    pair = scm_cons(scm_car(o), scm_null);
                    o = scm_cdr(o);
                }

                if (head == scm_null) {
                    head = pair; 
                }
                else {
                    scm_set_cdr(tail, pair);
                }
                tail = pair;

                /* address the rest */
                while (o != scm_null) {
                    pair = scm_cons(scm_car(o), scm_null);
                    scm_set_cdr(tail, pair);
                    tail = pair;
                    o = scm_cdr(o);
                }
            }

            exp = scm_cdr(exp);
        }
        if (exp != scm_null) {
            /* NOTE: here doesn't support unquote-splicing */
            o = eval_qq_ex(exp, env, 0, &sn);
            if (head == scm_null) {
                head = o; 
            }
            else {
                scm_set_cdr(tail, o);
            }
        }
        return head;
    }
    /* treat as ordinary quote */
    return unwrap_esymbols(exp);
}

static scm_object *eval_qq(scm_object *exp, scm_object *env) {
    int sp_len;
    return eval_qq_ex(scm_exp_get_qq_exp(exp), env, 0, &sp_len);
}

static void bind_keyword(scm_object *binding, scm_object *nenv, scm_object *xformer_env) {
    scm_object *kw = scm_exp_get_binding_keyword(binding);
    scm_object *spec = scm_exp_get_binding_spec(binding);
    scm_object *literals = scm_exp_get_spec_literals(spec);
    scm_object *rules = scm_exp_get_spec_rules(spec);
    scm_object *transformer = scm_transformer_new(kw, literals, rules, xformer_env);
    scm_env_define_var(nenv, kw, transformer);
}

/* return the new env after extension */
static scm_object *bind_keywords(scm_object *exp, scm_object *env, int rec) {
    scm_object *bindings = scm_exp_get_syntax_bindings(exp);
    scm_object *nenv = scm_env_extend(env, scm_null, scm_null);
    scm_object *binding;
    scm_object *xformer_env = rec ? nenv : env;
    while (!scm_exp_is_syntax_empty_bindings(bindings)) {
        binding = scm_exp_get_syntax_first_binding(bindings);
        bind_keyword(binding, nenv, xformer_env);
        bindings = scm_exp_get_syntax_rest_bindings(bindings);
    }
    return nenv;
}

static scm_object *eval_let_syntax(scm_object *exp, scm_object *env) {
    scm_exp_check_let_syntax(exp);
    scm_object *nenv = bind_keywords(exp, env, 0);
    return scm_eval_sequence(scm_exp_get_let_syntax_body(exp), nenv);
}

static scm_object *eval_letrec_syntax(scm_object *exp, scm_object *env) {
    scm_object *nenv = bind_keywords(exp, env, 1);
    return scm_eval_sequence(scm_exp_get_let_syntax_body(exp), nenv);
}

static scm_object *eval_define_syntax(scm_object *exp, scm_object *env) {
    if (env != scm_global_env())
        scm_error_object(exp, "define-syntax: only valid at the top level of a <program>");
    scm_object *binding = scm_exp_get_define_syntax_binding(exp);
    bind_keyword(binding, env, env);
    return scm_void;
}

static scm_object *eval_operands(scm_object *exp, scm_object *env, int *n) {
    scm_object *head = scm_null;
    scm_object *tail = NULL;
    scm_object *obj = NULL;
    scm_object *pair = NULL;
    int i = 0;
    scm_object *opds = scm_exp_get_application_operands(exp);
    while (opds->type == scm_type_pair) {
        obj = scm_eval(scm_car(opds), env);
        pair = scm_cons(obj, scm_null);
        if (i == 0) {
            head = pair;
        }
        else {
            scm_set_cdr(tail, pair);
        }
        tail = pair;
        opds = scm_cdr(opds);
        ++i;
    }
    *n = i;
    return head;
}

static scm_object *eval_core_syntax(scm_object *o, scm_object *exp, scm_object *env) {
    scm_core_syntax *syntax = (scm_core_syntax *)o;
    return syntax->eval(exp, env);
}

static scm_object *eval_syntax(scm_object *o, scm_object *exp, scm_object *env) {
    switch (o->type) {
    case scm_type_core_syntax:
        scm_exp_check_syntax(exp, scm_symbol_get_string(((scm_core_syntax*)o)->kw));
        return eval_core_syntax(o, exp, env);
        break;
    /* macro expression can be an improper list, so we don't check */
    default:    /* transformer */
        return scm_eval(transform_macro(o, exp, env), env);
    }
}

static scm_object *eval_application(scm_object *opt, scm_object *exp, scm_object *env) {
    scm_exp_check_application(exp);
    if (!opt || (opt->type != scm_type_primitive && opt->type != scm_type_compound))
        scm_error_object(exp, "#%%app: not a procedure;\nexpected a procedure "
                         "that can be applied to arguments\ngiven: ");

    int n = 0;
    scm_object *opds = eval_operands(exp, env, &n);
    
    return scm_apply(opt, n, opds);
}

static scm_object *eval_syntax_or_application(scm_object *exp, scm_object *env) {
    scm_object *o = NULL, *opt = NULL;
    opt = scm_exp_get_application_operator(exp);

    if (IS_IDENTIFIER(opt)) {
        o = eval_variable(opt, env);
        if (o->type == scm_type_core_syntax || o->type == scm_type_transformer)
            return eval_syntax(o, exp, env);
    }

    return eval_application(o, exp, env);
}


scm_object *scm_eval(scm_object *exp, scm_object *env) {
    if (scm_exp_is_self_evaluation(exp))
        return exp;
    if (IS_VECTOR(exp))
        return unwrap_esymbols(exp); /* treat vector as a quote by default */
    if (IS_IDENTIFIER(exp)) {
        scm_object *o = eval_variable(exp, env);
        if (o->type == scm_type_core_syntax || o->type == scm_type_transformer)
            variable_error(exp);
        return o;
    }
    if (scm_exp_is_syntax_or_application(exp)) {
        return eval_syntax_or_application(exp, env);
    }
    if (exp == scm_null)
        scm_error_object(exp, "#%%app: missing procedure expression in: ");

    scm_error_object(exp, "eval: bad syntax in: ");
    return NULL;
}

scm_object *scm_apply(scm_object *opt, int n, scm_object *opds) {
    scm_procedure_check_arity(opt, n);
    if (opt->type == scm_type_primitive) {
        scm_procedure_check_contract(opt, opds);
        return scm_primitive_apply(opt, n, opds);
    }
    else {
        return scm_compound_apply(opt, opds);
    }
}

static scm_object_methods core_syntax_methods = { core_syntax_free, same_object, same_object };

static int initialized = 0;

int scm_eval_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_core_syntax, &core_syntax_methods);

    initialized = 1;
    return 0;
}

int scm_eval_init_env(scm_object *env) {
    scm_env_define_var(env, sym_quote, core_syntax_new(sym_quote, eval_quote));
    scm_env_define_var(env, sym_set, core_syntax_new(sym_set, eval_assignment));
    scm_env_define_var(env, sym_lambda, core_syntax_new(sym_lambda, eval_lambda));
    scm_env_define_var(env, sym_define, core_syntax_new(sym_define, eval_definition));
    scm_env_define_var(env, sym_if, core_syntax_new(sym_if, eval_if));
    scm_env_define_var(env, sym_begin, core_syntax_new(sym_begin, eval_begin));
    scm_env_define_var(env, sym_quasiquote, core_syntax_new(sym_quasiquote, eval_qq));
    scm_env_define_var(env, sym_let_syntax, core_syntax_new(sym_let_syntax, eval_let_syntax));
    scm_env_define_var(env, sym_letrec_syntax, core_syntax_new(sym_letrec_syntax, eval_letrec_syntax));
    scm_env_define_var(env, sym_define_syntax, core_syntax_new(sym_define_syntax, eval_define_syntax));
    scm_env_define_var(env, sym_unquote, core_syntax_new(sym_unquote, eval_unquote));
    scm_env_define_var(env, sym_unquote_splicing, core_syntax_new(sym_unquote_splicing, eval_unquote_splicing));

    return 0;
}
