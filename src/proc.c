#include "proc.h"
#include "err.h"
#include "env.h"
#include "eval.h"

#include <stdlib.h>

typedef struct scm_primitive_st {
    scm_object base;
    int min_arity;
    int max_arity;
    const char *name;
    prim_fn fn;
    /* contract for the parameters.
     * if is not a pair, it's used for all the rest arguments */
    scm_object *preds;
} scm_primitive;

typedef struct scm_compound_st {
    scm_object base;
    int min_arity;
    int max_arity;
    const char *name;
    /* the order of the above fields must be same as the scm_primitive */
    scm_object *params;
    scm_object *body;
    scm_object *env;
} scm_compound;

static void arity_mismatch(scm_primitive *proc, int n) {
    char *str = " ";
    if (proc->max_arity == -1)
        str = " at least ";
    scm_error("%s: arity mismatch;\nthe expected number of arguments does not "
              "match the given number\nexpected:%s%d\ngiven: %d", proc->name,
              str, proc->min_arity, n);
}

static void contract_violation(scm_object *opt, int n, scm_object *pred, scm_object *opd) {
    scm_error_object(opd, "%s: contract violation by argument #%d\nexpected: %s\ngiven: ",
                     ((scm_primitive *)opt)->name, n, ((scm_primitive *)pred)->name);
}

void scm_primitive_free(scm_object *obj) {
    (void)obj;
}

scm_object *scm_primitive_new(const char *name, prim_fn fn, int min_arity,
                              int max_arity, scm_object *preds) {
    scm_primitive *prim = malloc(sizeof(scm_primitive));
    prim->base.type = scm_type_primitive;
    prim->fn = fn;
    prim->name = name;
    prim->min_arity = min_arity;
    prim->max_arity = max_arity;
    prim->preds = preds;

    return (scm_object *)prim;
}

scm_object *scm_compound_new(scm_object *params, scm_object *body, scm_object *env) {
    scm_compound *comp = malloc(sizeof(scm_compound));
    comp->base.type = scm_type_compound;
    comp->params = params;
    comp->body = body;
    comp->env = env;

    return (scm_object *)comp;
}

static void check_arity(scm_object *opt, int n) {
    scm_primitive *proc = (scm_primitive *)opt;
    if (n < proc->min_arity || (proc->max_arity != -1 && n > proc->max_arity))
        arity_mismatch(proc, n);
}

static scm_object *scm_primitive_apply(scm_object *opt, int n, scm_object *opds) {
    scm_primitive *proc = (scm_primitive *)opt;
    return proc->fn(n, opds);
}

static scm_object *scm_compound_apply(scm_object *opt, scm_object *opds) {
    scm_compound *proc = (scm_compound *)opt;
    scm_object *env = scm_env_extend(proc->env, proc->params, opds);
    return eval_sequence(proc->body, env);
}

static void check_contract(scm_object *opt, scm_object *opds) {
    if (opt->type == scm_type_compound)
        return;
    scm_primitive *proc = (scm_primitive *)opt;
    scm_object *preds = proc->preds;
    scm_object *pred = NULL;
    int i = 1;
    if (preds) {
        while (preds->type == scm_type_pair) {
            pred = scm_car(preds);
            if (scm_primitive_apply(pred, 1, opds) == scm_false)
                contract_violation(opt, i, pred, scm_car(opds));
            preds = scm_cdr(preds);
            opds = scm_cdr(opds);
            ++i;
        }
        if (preds != scm_null) {
            while (opds != scm_null) {
                if (scm_primitive_apply(preds, 1, opds) == scm_false)
                    contract_violation(opt, i, preds, scm_car(opds));
                opds = scm_cdr(opds);
                ++i;
            }
        }
    }
}

scm_object *scm_apply(scm_object *opt, int n, scm_object *opds) {
    check_arity(opt, n);
    if (opt->type == scm_type_primitive) {
        check_contract(opt, opds);
        return scm_primitive_apply(opt, n, opds);
    }
    else {
        return scm_compound_apply(opt, opds);
    }
}
