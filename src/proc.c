#include "proc.h"
#include "err.h"
#include "env.h"
#include "eval.h"

#include <string.h>
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
    char *name;
    /* the order of the above fields must be same as the scm_primitive */
    scm_object *params;
    scm_object *body;
    scm_object *env;
} scm_compound;

static void arity_mismatch(scm_primitive *proc, int n) {
    int min = proc->min_arity;
    int max = proc->max_arity;
    const char *name = proc->name;
    const char *str = "";
    if (name)
        str = ": ";
    else
        name = "";
    if (min == max) {
        scm_error("%s%sarity mismatch;\nthe expected number of arguments does "
                  "not match the given number\nexpected: %d\ngiven: %d",
                  name, str, min, n);
    }
    else if (proc->max_arity == -1) {
        scm_error("%s%sarity mismatch;\nthe expected number of arguments does "
                  "not match the given number\nexpected: at least %d\ngiven: "
                  "%d", name, str, min, n);
    }
    else {
        scm_error("%s%sarity mismatch;\nthe expected number of arguments does "
                  "not match the given number\nexpected: [%d, %d]\ngiven: %d",
                  name, str, min, max, n);
    }
}

static void contract_violation(scm_object *opt, int n, scm_object *pred, scm_object *opd) {
    scm_error_object(opd, "%s: contract violation by argument #%d\nexpected: %s\ngiven: ",
                     ((scm_primitive *)opt)->name, n, ((scm_primitive *)pred)->name);
}

static void primitive_free(scm_object *obj) {
    free(obj);
}

static void compound_free(scm_object *obj) {
    scm_compound *comp = (scm_compound *)obj;
    if (comp->name)
        free(comp->name);
    free(comp);
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
    comp->max_arity = scm_list_length(params);
    if (comp->max_arity == -1)
        comp->min_arity = scm_list_quasilength(params);
    else
        comp->min_arity = comp->max_arity;

    comp->params = params;
    comp->body = body;
    comp->env = env;

    return (scm_object *)comp;
}

const char *scm_procedure_name(scm_object *proc) {
    return ((scm_primitive *)proc)->name;
}

void scm_compound_set_name(scm_object *proc, const char *name) {
    scm_compound *p = (scm_compound *)proc;
    /* only set the name when the procedure is first defined */
    if (!p->name) {
        int len = strlen(name);
        p->name = malloc(len + 1);
        strncpy(p->name, name, len+1);
    }
}

void scm_procedure_check_arity(scm_object *opt, int n) {
    scm_primitive *proc = (scm_primitive *)opt;
    if (n < proc->min_arity || (proc->max_arity != -1 && n > proc->max_arity))
        arity_mismatch(proc, n);
}

scm_object *scm_primitive_apply(scm_object *opt, int n, scm_object *opds) {
    scm_primitive *proc = (scm_primitive *)opt;
    return proc->fn(n, opds);
}

scm_object *scm_compound_apply(scm_object *opt, scm_object *opds) {
    scm_compound *proc = (scm_compound *)opt;
    scm_object *env = scm_env_extend(proc->env, proc->params, opds);
    return scm_eval_sequence(proc->body, env);
}

void scm_procedure_check_contract(scm_object *opt, scm_object *opds) {
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

static int proc_eqv(scm_object *o1, scm_object *o2) {
    return o1 == o2;
}

static scm_object_methods prim_methods = { primitive_free, proc_eqv, proc_eqv };
static scm_object_methods comp_methods = { compound_free, proc_eqv, proc_eqv };

static int initialized = 0;

int scm_proc_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_primitive, &prim_methods);
    scm_object_register(scm_type_compound, &comp_methods);

    initialized = 1;
    return 0;
}
