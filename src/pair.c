#include "pair.h"
#include "number.h"
#include "proc.h"
#include "env.h"

#include <stdarg.h>
#include <stdlib.h>

typedef struct scm_pair_st {
    scm_object base;
    scm_object *car;
    scm_object *cdr;
} scm_pair;

scm_object *scm_cons(scm_object *car, scm_object *cdr) {
    scm_pair *pair = malloc(sizeof(scm_pair));

    if (!pair) {
        return NULL;
    }

    pair->base.type = scm_type_pair;

    pair->car = car;
    pair->cdr = cdr;

    return (scm_object *)pair;
}
define_primitive_2(cons);

static void pair_free(scm_object *pair) {
    scm_pair *p = (scm_pair *)pair;
    scm_object_free(p->car);
    scm_object_free(p->cdr);

    free(pair);
}

scm_object *scm_car(scm_object *pair) {
    if (pair->type != scm_type_pair) 
        return NULL;    /* contract violation */

    scm_pair *p = (scm_pair *)pair;
    return p->car;
}
define_primitive_1(car);

scm_object *scm_cdr(scm_object *pair) {
    if (pair->type != scm_type_pair) 
        return NULL;    /* contract violation */

    scm_pair *p = (scm_pair *)pair;
    return p->cdr;
}
define_primitive_1(cdr);

scm_object *scm_set_car(scm_object *pair, scm_object *o) {
    scm_pair *p = (scm_pair *)pair;
    /* not free p->car here, may be referenced by others, leave it to gc to consider */
    p->car = o;
    return scm_void;
}
define_primitive_2(set_car);

scm_object *scm_set_cdr(scm_object *pair, scm_object *o) {
    scm_pair *p = (scm_pair *)pair;
    p->cdr = o;
    return scm_void;
}
define_primitive_2(set_cdr);

static int pair_equal(scm_object *o1, scm_object *o2) {
    if (same_object(o1, o2))
        return 1;
    scm_object *a1 = scm_car(o1);
    scm_object *a2 = scm_car(o2);
    scm_object *d1 = scm_cdr(o1);
    scm_object *d2 = scm_cdr(o2);
    return scm_equal(a1, a2) && scm_equal(d1, d2);
}

/* create a list */
scm_object *scm_list(long count, ...) {
    va_list objs;
    va_start(objs, count);

    scm_object *head = scm_null;
    scm_object *tail = NULL;
    scm_object *pair = NULL;
    scm_object *obj = NULL;

    for (long i = 0; i < count ; ++i) {
        obj = va_arg(objs, scm_object*);
        pair = scm_cons(obj, scm_null); 

        if (i == 0) {
            head = pair;
        }
        else {
            scm_set_cdr(tail, pair);
        }
        tail = pair;
    }

    va_end(objs);

    return head;
}

static scm_object *prim_list(int n, scm_object *li) {
    (void)n;
    return li;
}

scm_object *scm_list_ref(scm_object *list, long k) {
    scm_object *p = list;

    if (k < 0) {
        return NULL;    /* invalid index */
    }

    while (k--) {
        p = scm_cdr(p);
    }

    return scm_car(p);
}

/* the resulting list is not newly allocated, in contrast to append */
scm_object *scm_list_combine(scm_object *l1, scm_object *l2) {
    if (l1 == scm_null)
        return l2;
    if (l2 == scm_null)
        return l1;

    scm_object *last = scm_list_last_pair(l1);
    scm_set_cdr(last, l2);

    return l1;
}

scm_object *scm_list_last_pair(scm_object *l) {
    scm_object *d;
    if (!IS_PAIR(l))
        return l;
    while (1) {
        d = scm_cdr(l);
        if (IS_PAIR(d))
            l = d;
        else
            return l;
    }
}

/* TODO: solve cycle list */
long scm_list_length(scm_object *list) {
    long i = 0;
    scm_object *p = list;
    while (p != scm_null) {
        if (p->type != scm_type_pair) {
            return -1;  /* not a list */
        }
        p = scm_cdr(p);
        ++i;
    }

    return i;
}

/* treat the last cdr as scm_null */
long scm_list_quasilength(scm_object *list) {
    long i = 0;
    scm_object *p = list;
    while (p ->type == scm_type_pair) {
        p = scm_cdr(p);
        ++i;
    }

    return i;
}

static int scm_mem_ex(scm_object *o, scm_object *l, scm_eq_fn eq) {
    while (l != scm_null) {
        if (eq(o, scm_car(l)))
            return 1;
        l = scm_cdr(l);
    }
    return 0;
}

int scm_memq(scm_object *o, scm_object *l) {
    return scm_mem_ex(o, l, scm_eq);
}

int scm_memv(scm_object *o, scm_object *l) {
    return scm_mem_ex(o, l, scm_eqv);
}

int scm_member(scm_object *o, scm_object *l) {
    return scm_mem_ex(o, l, scm_equal);
}

static scm_object *scm_ass_ex(scm_object *o, scm_object *l, scm_eq_fn eq) {
    while (l != scm_null) {
        if (eq(o, scm_caar(l)))
            return scm_car(l);
        l = scm_cdr(l);
    }
    return scm_false;
}

scm_object *scm_assq(scm_object *o, scm_object *l) {
    return scm_ass_ex(o, l, scm_eq);
}

scm_object *scm_assv(scm_object *o, scm_object *l) {
    return scm_ass_ex(o, l, scm_eqv);
}

scm_object *scm_assoc(scm_object *o, scm_object *l) {
    return scm_ass_ex(o, l, scm_equal);
}

static scm_object *scm_is_list(scm_object *list) {
    return scm_boolean(scm_list_length(list) != -1);
}
define_primitive_1(is_list);

scm_object *scm_caar(scm_object *pair) {
    return scm_car(scm_car(pair));
}

scm_object *scm_cadr(scm_object *pair) {
    return scm_car(scm_cdr(pair));
}

scm_object *scm_cdar(scm_object *pair) {
    return scm_cdr(scm_car(pair));
}

scm_object *scm_cddr(scm_object *pair) {
    return scm_cdr(scm_cdr(pair));
}

scm_object *scm_caaar(scm_object *pair) {
    return scm_car(scm_car(scm_car(pair)));
}

scm_object *scm_caadr(scm_object *pair) {
    return scm_car(scm_car(scm_cdr(pair)));
}

scm_object *scm_cadar(scm_object *pair) {
    return scm_car(scm_cdr(scm_car(pair)));
}

scm_object *scm_caddr(scm_object *pair) {
    return scm_car(scm_cdr(scm_cdr(pair)));
}

scm_object *scm_cdaar(scm_object *pair) {
    return scm_cdr(scm_car(scm_car(pair)));
}

scm_object *scm_cdadr(scm_object *pair) {
    return scm_cdr(scm_car(scm_cdr(pair)));
}

scm_object *scm_cddar(scm_object *pair) {
    return scm_cdr(scm_cdr(scm_car(pair)));
}

scm_object *scm_cdddr(scm_object *pair) {
    return scm_cdr(scm_cdr(scm_cdr(pair)));
}

scm_object *scm_caaaar(scm_object *pair) {
    return scm_car(scm_car(scm_car(scm_car(pair))));
}

scm_object *scm_caaadr(scm_object *pair) {
    return scm_car(scm_car(scm_car(scm_cdr(pair))));
}

scm_object *scm_caadar(scm_object *pair) {
    return scm_car(scm_car(scm_cdr(scm_car(pair))));
}

scm_object *scm_caaddr(scm_object *pair) {
    return scm_car(scm_car(scm_cdr(scm_cdr(pair))));
}

scm_object *scm_cadaar(scm_object *pair) {
    return scm_car(scm_cdr(scm_car(scm_car(pair))));
}

scm_object *scm_cadadr(scm_object *pair) {
    return scm_car(scm_cdr(scm_car(scm_cdr(pair))));
}

scm_object *scm_caddar(scm_object *pair) {
    return scm_car(scm_cdr(scm_cdr(scm_car(pair))));
}

scm_object *scm_cadddr(scm_object *pair) {
    return scm_car(scm_cdr(scm_cdr(scm_cdr(pair))));
}

scm_object *scm_cdaaar(scm_object *pair) {
    return scm_cdr(scm_car(scm_car(scm_car(pair))));
}

scm_object *scm_cdaadr(scm_object *pair) {
    return scm_cdr(scm_car(scm_car(scm_cdr(pair))));
}

scm_object *scm_cdadar(scm_object *pair) {
    return scm_cdr(scm_car(scm_cdr(scm_car(pair))));
}

scm_object *scm_cdaddr(scm_object *pair) {
    return scm_cdr(scm_car(scm_cdr(scm_cdr(pair))));
}

scm_object *scm_cddaar(scm_object *pair) {
    return scm_cdr(scm_cdr(scm_car(scm_car(pair))));
}

scm_object *scm_cddadr(scm_object *pair) {
    return scm_cdr(scm_cdr(scm_car(scm_cdr(pair))));
}

scm_object *scm_cdddar(scm_object *pair) {
    return scm_cdr(scm_cdr(scm_cdr(scm_car(pair))));
}

scm_object *scm_cddddr(scm_object *pair) {
    return scm_cdr(scm_cdr(scm_cdr(scm_cdr(pair))));
}

static scm_object_methods pair_methods = { pair_free, same_object, pair_equal };

static int initialized = 0;

int scm_pair_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_pair, &pair_methods);

    initialized = 1;
    return 0;
}

int scm_pair_init_env(scm_object *env) {
    scm_env_add_prim(env, "cons", prim_cons, 2, 2, NULL);
    scm_env_add_prim(env, "car", prim_car, 1, 1, pred_pair);
    scm_env_add_prim(env, "cdr", prim_cdr, 1, 1, pred_pair);
    scm_env_add_prim(env, "set-car!", prim_set_car, 2, 2, scm_list(1, pred_pair));
    scm_env_add_prim(env, "set-cdr!", prim_set_cdr, 2, 2, scm_list(1, pred_pair));
    scm_env_add_prim(env, "list", prim_list, 0, -1, NULL);
    scm_env_add_prim(env, "list?", prim_is_list, 1, 1, NULL);

    return 0;
}
