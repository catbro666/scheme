#include "pair.h"

#include <stdarg.h>
#include <stdlib.h>

typedef struct scm_pair_st {
    scm_object base;
    scm_object *car;
    scm_object *cdr;
} scm_pair;

static scm_object scm_null_arr[1] = {{scm_type_null}};
scm_object *scm_null = scm_null_arr;

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

scm_object *scm_cdr(scm_object *pair) {
    if (pair->type != scm_type_pair) 
        return NULL;    /* contract violation */

    scm_pair *p = (scm_pair *)pair;
    return p->cdr;
}

void scm_set_car(scm_object *pair, scm_object *o) {
    scm_pair *p = (scm_pair *)pair;
    /* not free p->car here, may be referenced by others, leave it to gc to consider */
    p->car = o;
}

void scm_set_cdr(scm_object *pair, scm_object *o) {
    scm_pair *p = (scm_pair *)pair;
    p->cdr = o;
}

static int pair_eqv(scm_object *o1, scm_object *o2) {
    return o1 == o2;
}

static int pair_equal(scm_object *o1, scm_object *o2) {
    if (pair_eqv(o1, o2))
        return 1;
    scm_object *a1 = scm_car(o1);
    scm_object *a2 = scm_car(o2);
    scm_object *d1 = scm_cdr(o1);
    scm_object *d2 = scm_cdr(o2);
    return scm_object_equal(a1, a2) && scm_object_equal(d1, d2);
}

/* create a list */
scm_object *scm_list(int count, ...) {
    va_list objs;
    va_start(objs, count);

    scm_object *head = scm_null;
    scm_object *tail = NULL;
    scm_object *pair = NULL;
    scm_object *obj = NULL;

    for (int i = 0; i < count ; ++i) {
        obj = va_arg(objs, scm_object*);
        pair = scm_cons(obj, scm_null); 
        if (!pair)
            goto err;

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
err:
    if (head) {
        pair_free(head);
    }
    return NULL;
}

scm_object *scm_list_ref(scm_object *list, int k) {
    scm_object *p = list;

    if (k < 0) {
        return NULL;    /* invalid index */
    }

    while (k--) {
        p = scm_cdr(p); 
    }

    return scm_car(p);
}

/* TODO: solve cycle list
 * return -1 when the parameter is not a list */
int scm_list_length(scm_object *list) {
    int i = 0;
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

int scm_is_list(scm_object *list) {
    return scm_list_length(list) != -1;
}

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

static scm_object_methods pair_methods = { pair_free, pair_eqv, pair_equal };

static int initialized = 0;

int scm_pair_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_pair, &pair_methods);
    scm_object_register(scm_type_null, &simple_methods);

    initialized = 1;
    return 0;
}
