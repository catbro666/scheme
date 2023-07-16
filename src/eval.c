#include "eval.h"

#include "err.h"
#include "symbol.h"
#include "pair.h"
#include "vector.h"
#include "proc.h"
#include "env.h"
#include "exp.h"

#include <stdlib.h>

static void unquote_splicing_error(scm_object *exp, char *err) {
    scm_error_object(exp, "unquote-splicing: %s in: ", err);
}

static scm_object *eval_variable(scm_object *exp, scm_object *env) {
    scm_object *o = scm_env_lookup_var(env, exp);
    if (o == NULL) {
        char *str = scm_symbol_get_string(exp);
        scm_error("%s: undefined;\ncannot reference an identifier before its definition", str);
    }
    return o;    
}

static scm_object *eval_assignment(scm_object *exp, scm_object *env) {
    int res = scm_env_set_var(env, scm_exp_get_assignment_var(exp),
                              scm_exp_get_assignment_val(exp));
    if (res)
        scm_error_object(exp, "set!: assignment disallowed;\ncan't set "
                         "variable before its definition\nvariable: ");
    return scm_void;
}

static scm_object *eval_lambda(scm_object *exp, scm_object *env) {
    return scm_compound_new(scm_exp_get_lambda_parameters(exp),
                            scm_exp_get_lambda_body(exp), env);
}

static scm_object *eval_definition(scm_object *exp, scm_object *env) {
    scm_env_define_var(env, scm_exp_get_definition_var(exp),
                       scm_exp_get_definition_val(exp));
    return scm_void;
}

static scm_object *eval_if(scm_object *exp, scm_object *env) {
    if (scm_false != scm_eval(scm_exp_get_if_test(exp), env))
        return scm_eval(scm_exp_get_if_consequent(exp), env);
    else
        return scm_eval(scm_exp_get_if_alternate(exp), env);
}

scm_object *eval_sequence(scm_object *exp, scm_object *env) {
    if (scm_exp_is_sequence_last(exp)) {
        /* tail call */
        return scm_eval(scm_exp_get_sequence_first(exp), env);
    }
    else {
        scm_eval(scm_exp_get_sequence_first(exp), env);
        return eval_sequence(scm_exp_get_sequence_rest(exp), env);
    }
}

/* @in_seq: indicate if the current exp is in sequence context
 * @sp_len: out parameter, indicate if the current exp is unquote-splicing
 *          returns the length of list, otherwise returns -1 */
static scm_object *eval_qq_ex(scm_object *exp, scm_object *env, int in_seq, int *sp_len) {
    scm_object *e = NULL;
    scm_object *o = NULL;
    *sp_len = -1;

    if (scm_exp_is_unquote(exp))
        return scm_eval(scm_exp_get_unquote_exp(exp), env);
    if (scm_exp_is_unquote_splicing(exp)) {
        if (!in_seq)
            unquote_splicing_error(exp, "not in context of sequence");
        o = scm_eval(scm_exp_get_unquote_splicing_exp(exp), env);
        *sp_len = scm_list_length(o);
        if (*sp_len < 0)
            unquote_splicing_error(exp, "not returning a list");

        return o;
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
        while (exp->type == scm_type_pair && !scm_exp_is_unquote(exp)
               && !scm_exp_is_unquote_splicing(exp)) {
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
    return exp;
}

static scm_object *eval_qq(scm_object *exp, scm_object *env) {
    int sp_len;
    return eval_qq_ex(exp, env, 0, &sp_len);
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

static scm_object *eval_application(scm_object *exp, scm_object *env) {
    scm_object *opt = scm_eval(scm_exp_get_application_operator(exp), env);
    if (opt->type != scm_type_primitive && opt->type != scm_type_compound)
        scm_error_object(opt, "#%%app: not a procedure;\nexpected a procedure "
                         "that can be applied to arguments\ngiven: ");

    int n = 0;
    scm_object *opds = eval_operands(exp, env, &n);
    
    return scm_apply(opt, n, opds);
}

scm_object *scm_eval(scm_object *exp, scm_object *env) {
    if (scm_exp_is_self_evaluation(exp))
        return exp;
    if (scm_exp_is_variable(exp))
        return eval_variable(exp, env);
    if (scm_exp_is_quote(exp))
        return scm_exp_get_quote_text(exp);
    if (scm_exp_is_assignment(exp))
        return eval_assignment(exp, env);
    if (scm_exp_is_lambda(exp))
        return eval_lambda(exp, env);
    if (scm_exp_is_definition(exp))
        return eval_definition(exp, env);
    if (scm_exp_is_if(exp))
        return eval_if(exp, env);
    if (scm_exp_is_begin(exp))
        return eval_sequence(scm_exp_get_begin_sequence(exp), env);
    if (scm_exp_is_qq(exp))
        return eval_qq(scm_exp_get_qq_exp(exp), env);
    if (scm_exp_is_unquote(exp))
        scm_error_object(exp, "unquote: not in quasiquote in: ");
    if (scm_exp_is_unquote_splicing(exp))
        scm_error_object(exp, "unquote-splicing: not in quasiquote in: ");
    if (scm_exp_is_application(exp))
        return eval_application(exp, env);

    scm_error_object(exp, "eval: bad syntax in: ");
    return NULL;
}
