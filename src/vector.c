#include "vector.h"
#include <stdarg.h>
#include <stdlib.h>

typedef struct scm_vector_st {
    scm_object base;
    scm_object **elts;
    int len;
} scm_vector;

static scm_object *empty_vector = NULL;

static scm_vector *scm_vector_alloc(count) {
    scm_vector *vec = calloc(1, sizeof(scm_vector));
    vec->base.type = scm_type_vector;
    vec->len = count;
    if (count > 0) {
        vec->elts = calloc(count, sizeof(scm_object *));
    }

    return vec;
}

scm_object *scm_vector_new(int count, ...) {
    va_list objs;
    scm_object *obj;

    if (count < 0) {
        return NULL;    /* invalid index */
    }
    if (count == 0)
        return empty_vector;

    scm_vector *vec = scm_vector_alloc(count);

    va_start(objs, count);

    for (int i = 0; i < count ; ++i) {
        obj = va_arg(objs, scm_object *);
        vec->elts[i] = obj;
    }

    return (scm_object *)vec;
}

void scm_vector_fill(scm_object *obj, scm_object *fill) {
    scm_vector *vec = (scm_vector *)obj;
    int i = vec->len;

    while (i--) {
        vec->elts[i] = fill; /* not free the old */
    }
}

scm_object *scm_vector_new_fill(int count, scm_object *fill) {
    scm_object *vec = (scm_object *)scm_vector_alloc(count);

    scm_vector_fill(vec, fill);
    
    return vec;
}

static void scm_vector_free(scm_object *obj) {
    scm_vector *vec = (scm_vector *)obj;
    int i = vec->len;

    if (i) {
        while (i--) {
            if (vec->elts[i])
                scm_object_free(vec->elts[i]);
        }
        free(vec);
    }
}

scm_object *scm_vector_ref(scm_object *obj, int k) {
    scm_vector *vec = (scm_vector *)obj;

    if (k >= vec->len) {
        return NULL;    /* invalid index */
    }

    return vec->elts[k];
}

int scm_vector_length(scm_object *obj) {
    scm_vector *vec = (scm_vector *)obj;
    return vec->len;
}

static int initialized = 0;

int scm_vector_env_init(void) {
    if (initialized) return 0;

    empty_vector = (scm_object *)scm_vector_alloc(0);

    scm_object_register(scm_type_vector, scm_vector_free);

    initialized = 1;
    return 0;
}
