#include "string.h"
#include "err.h"
#include "char.h"
#include "number.h"
#include "proc.h"
#include "env.h"

#include <stdlib.h>
#include <string.h>

typedef struct scm_string_st {
    scm_object base;
    char *buf;
    long len;   /* NOTE: need proper type */
} scm_string;

static scm_object *empty_string = NULL;

static void out_of_range(scm_object *obj, long k) {
    scm_string *str = (scm_string *)obj;
    if (obj == empty_string)
        scm_error_object(obj, "string-ref: index is out of range for "
                         "empty string\nindex: %ld\nstring: ", k);
    else
        scm_error_object(obj, "string-ref: index is out of range\nindex: %ld\n"
                         "valid range: [0, %ld]\nstring: ", k, str->len-1);
}

static scm_object *string_alloc(char *buf, long len) {
    scm_string *str = malloc(sizeof(scm_string));

    str->base.type = scm_type_string;

    str->buf = buf;
    str->len = len;

    return (scm_object *)str;
}

/* scm_string_new doesn't copy buf */
scm_object *scm_string_new(char *buf, long len) {
    if (len < 0) {
        len = strlen(buf);
    }

    if (len == 0)
        return empty_string;
    else
        return string_alloc(buf, len);
}

/* scm_string_copy_new copies buf, for tests */
scm_object *scm_string_copy_new(const char *buf, long len) {
    if (len < 0) {
        len = strlen(buf);
    }

    if (len == 0)
        return empty_string;
    else {
        char *new_buf = malloc(len + 1);
        strncpy(new_buf, buf, len);
        new_buf[len] = '\0';
        return string_alloc(new_buf, len);
    }
}

static void string_free(scm_object *obj) {
    if (obj == empty_string)
        return;

    scm_string *s = (scm_string *)obj;
    free(s->buf);
    free(s);
}

long scm_string_length(scm_object *obj) {
    scm_string *s = (scm_string *)obj;
    return s->len;
}

static scm_object *prim_string_length(int n, scm_object *args) {
    (void)n;
    return INTEGER(scm_string_length(scm_car(args)));
}

scm_object *scm_string_ref(scm_object *obj, long k) {
    scm_string *s = (scm_string *)obj;
    if (k < 0 || k >= s->len) {
        out_of_range(obj, k);
    }
    return (scm_object *)scm_chars[(int)scm_string_get_char(obj, k)];
}

static scm_object *prim_string_ref(int n, scm_object *args) {
    (void)n;
    return scm_string_ref(scm_car(args), scm_integer_get_val(scm_cadr(args)));
}

char scm_string_get_char(scm_object *obj, long k) {
    scm_string *s = (scm_string *)obj;
    return s->buf[k];
}

static int string_eqv(scm_object *o1, scm_object *o2) {
    return o1 == o2;
}

static int string_equal(scm_object *o1, scm_object *o2) {
    scm_string *s1 = (scm_string *)o1;
    scm_string *s2 = (scm_string *)o2;
    return string_eqv(o1, o2) ||
           (s1->len == s2->len && !strncmp(s1->buf, s2->buf, s1->len));
}

static scm_object_methods string_methods = { string_free, string_eqv, string_equal };

static int initialized = 0;
int scm_string_init(void) {
    if (initialized) return 0;

    empty_string = string_alloc(NULL, 0);

    scm_object_register(scm_type_string, &string_methods);

    initialized = 1;
    return 0;
}

int scm_string_init_env(scm_object *env) {
    scm_env_add_prim(env, "string-length", prim_string_length, 1, 1, pred_string);
    scm_env_add_prim(env, "string-ref", prim_string_ref, 2, 2, scm_list(2, pred_string, pred_integer));

    return 0;
}
