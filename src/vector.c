#include "vector.h"
#include "err.h"
#include "number.h"
#include "proc.h"
#include "env.h"

#include <stdarg.h>
#include <stdlib.h>

typedef struct scm_vector_st {
    scm_object base;
    scm_object **elts;
    int len;
} scm_vector;

scm_object *scm_empty_vector = NULL;

static void out_of_range(scm_object *obj, char *name, int k) {
    scm_vector *vec = (scm_vector *)obj;
    if (obj == scm_empty_vector)
        scm_error_object(obj, "%s: index is out of range for "
                         "empty vector\nindex: %d\nvector: ", name, k);
    else
        scm_error_object(obj, "%s: index is out of range\nindex: %d\n"
                         "valid range: [0, %d]\nvector: ", name, k, vec->len-1);
}

scm_object *scm_vector_alloc(int count) {
    scm_vector *vec = calloc(1, sizeof(scm_vector));
    vec->base.type = scm_type_vector;
    vec->len = count;
    if (count > 0) {
        vec->elts = calloc(count, sizeof(scm_object *));
    }

    return (scm_object *)vec;
}

scm_object *scm_vector_realloc(scm_object *vector, int count) {
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

    scm_vector *vec = (scm_vector *)scm_vector_alloc(count);

    va_start(objs, count);

    for (int i = 0; i < count ; ++i) {
        obj = va_arg(objs, scm_object *);
        vec->elts[i] = obj;
    }

    va_end(objs);
    return (scm_object *)vec;
}

static scm_object *prim_vector(int n, scm_object *args) {
    if (n == 0)
        return scm_empty_vector;
    scm_vector *vec = (scm_vector *)scm_vector_alloc(n);
    int i = 0;
    while (i < n) {
        vec->elts[i] = scm_car(args);
        args = scm_cdr(args);
        i++;
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
    scm_object *vec = scm_vector_alloc(count);

    scm_vector_fill(vec, fill);
    
    return vec;
}

static scm_object *scm_make_vector(int n, scm_object *k, scm_object *args) {
    scm_object *vec = scm_vector_alloc(scm_integer_get_val(k));
    if (n > 1)
        scm_vector_fill(vec, scm_car(args));

    return vec;
}
define_primitive_1n(make_vector);

static void vector_free(scm_object *vector) {
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
        out_of_range(vector, "vector-ref", k);
    }

    return vec->elts[k];
}

static scm_object *prim_vector_ref(int n, scm_object *args) {
    (void)n;
    return scm_vector_ref(scm_car(args), scm_integer_get_val(scm_cadr(args)));
}

scm_object *scm_vector_set(scm_object *vector, int k, scm_object *obj) {
    scm_vector *vec = (scm_vector *)vector;

    if (k < 0 || k >= vec->len) {
        out_of_range(vector, "vector-set!", k);
    }

    vec->elts[k] = obj;
    return scm_void;
}

static scm_object *prim_vector_set(int n, scm_object *args) {
    (void)n;
    return scm_vector_set(scm_car(args), scm_integer_get_val(scm_cadr(args)), scm_caddr(args));
}

int scm_vector_length(scm_object *vector) {
    scm_vector *vec = (scm_vector *)vector;
    return vec->len;
}

static scm_object *prim_vector_length(int n, scm_object *args) {
    (void)n;
    int len = scm_vector_length(scm_car(args));
    return INTEGER(len);
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

static int vector_eqv(scm_object *o1, scm_object *o2) {
    return o1 == o2;
}

static int vector_equal(scm_object *o1, scm_object *o2) {
    if (vector_eqv(o1, o2))
        return 1;
    scm_vector *v1 = (scm_vector *)o1;
    scm_vector *v2 = (scm_vector *)o2;
    if (v1->len != v2->len)
        return 0;
    for (int i = 0; i < v1->len; ++i) {
        if (!scm_equal(scm_vector_ref(o1, i), scm_vector_ref(o2, i)))
            return 0;
    }
    return 1;
}

static scm_object_methods vector_methods = { vector_free, vector_eqv, vector_equal };

static int initialized = 0;

int scm_vector_init(void) {
    if (initialized) return 0;

    scm_empty_vector = scm_vector_alloc(0);

    scm_object_register(scm_type_vector, &vector_methods);

    initialized = 1;
    return 0;
}

int scm_vector_init_env(scm_object *env) {
    scm_env_add_prim(env, "vector", prim_vector, 0, -1, NULL);
    scm_env_add_prim(env, "make-vector", prim_make_vector, 1, 2, scm_list(1, pred_integer));
    scm_env_add_prim(env, "vector-ref", prim_vector_ref, 2, 2, scm_list(2, pred_vector, pred_integer));
    scm_env_add_prim(env, "vector-set!", prim_vector_set, 3, 3, scm_list(2, pred_vector, pred_integer));
    scm_env_add_prim(env, "vector-length", prim_vector_length, 1, 1, pred_vector);

    return 0;
}
