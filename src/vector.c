#include "vector.h"
#include <stdarg.h>
#include <stdlib.h>

typedef struct scm_vector_st {
    scm_object base;
    scm_object **elts;
    int len;
} scm_vector;

scm_object *scm_empty_vector = NULL;

static scm_vector *scm_vector_alloc(count) {
    scm_vector *vec = calloc(1, sizeof(scm_vector));
    vec->base.type = scm_type_vector;
    vec->len = count;
    if (count > 0) {
        vec->elts = calloc(count, sizeof(scm_object *));
    }

    return vec;
}

static scm_object *scm_vector_realloc(scm_object *vector, int count) {
    scm_vector *vec = (scm_vector *)vector;
    void *new_etls = realloc(vec->elts, count * sizeof(scm_object *));
    if (!new_etls) {
        return NULL;
    }
    vec->elts = new_etls;
    vec->len = count;

    return vector;
}

scm_object *scm_vector_new(int count, ...) {
    va_list objs;
    scm_object *obj;

    if (count < 0) {
        return NULL;    /* invalid index */
    }
    if (count == 0)
        return scm_empty_vector;

    scm_vector *vec = scm_vector_alloc(count);

    va_start(objs, count);

    for (int i = 0; i < count ; ++i) {
        obj = va_arg(objs, scm_object *);
        vec->elts[i] = obj;
    }

    return (scm_object *)vec;
}

void scm_vector_fill(scm_object *vector, scm_object *fill) {
    scm_vector *vec = (scm_vector *)vector;
    int i = vec->len;

    while (i--) {
        vec->elts[i] = fill; /* not free the old, leave it to gc to consider */
    }
}

scm_object *scm_vector_new_fill(int count, scm_object *fill) {
    scm_object *vec = (scm_object *)scm_vector_alloc(count);

    scm_vector_fill(vec, fill);
    
    return vec;
}

static void scm_vector_free(scm_object *vector) {
    scm_vector *vec = (scm_vector *)vector;
    int i = vec->len;

    if (i) {
        while (i--) {
            if (vec->elts[i])
                scm_object_free(vec->elts[i]);
        }
        free(vec);
    }
}

scm_object *scm_vector_ref(scm_object *vector, int k) {
    scm_vector *vec = (scm_vector *)vector;

    if (k < 0 || k >= vec->len) {
        return NULL;    /* invalid index */
    }

    return vec->elts[k];
}

int scm_vector_set(scm_object *vector, int k, scm_object *obj) {
    scm_vector *vec = (scm_vector *)vector;

    if (k < 0 || k >= vec->len) {
        return 1;    /* invalid index */
    }

    vec->elts[k] = obj;
    return 0;
}

int scm_vector_length(scm_object *vector) {
    if (vector == scm_empty_vector)
        return 0;

    scm_vector *vec = (scm_vector *)vector;
    return vec->len;
}

scm_object *scm_vector_insert(scm_object *vector, scm_object *obj) {
    scm_object *vec = NULL;
    if (vector == scm_empty_vector) {
        vec = scm_vector_new(1, obj);
    }
    else {
        int len = scm_vector_length(vector);
        vec = scm_vector_realloc(vector, len + 1);
        scm_vector_set(vec, len, obj);
    }
    return vec;
}

static int initialized = 0;

int scm_vector_env_init(void) {
    if (initialized) return 0;

    scm_empty_vector = (scm_object *)scm_vector_alloc(0);

    scm_object_register(scm_type_vector, scm_vector_free);

    initialized = 1;
    return 0;
}
