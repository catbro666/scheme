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

static void scm_pair_free(scm_object *pair) {
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
    /* not free p->car here, may be referenced by others */
    p->car = o;
}

void scm_set_cdr(scm_object *pair, scm_object *o) {
    scm_pair *p = (scm_pair *)pair;
    p->cdr = o;
}

/* create a list */
scm_object *scm_list(int count, ...) {
    va_list objs;
    va_start(objs, count);

    scm_object *head = NULL;
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

    if (head)
        return head;
    else
        return scm_null;
err:
    if (head) {
        scm_pair_free(head);
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

/* TODO: solve cycle list */
int scm_list_length(scm_object *list) {
    int i = 0;
    scm_object *p = list;
    while (p != scm_null) {
        p = scm_cdr(p);
        ++i;
    }

    return i;
}

static int initialized = 0;

int scm_pair_env_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_pair, scm_pair_free);

    initialized = 1;
    return 0;
}
