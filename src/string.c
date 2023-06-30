#include "string.h"
#include "char.h"
#include <stdlib.h>
#include <string.h>

typedef struct scm_string_st {
    scm_object base;
    char *buf;
    int len;   /* NOTE: need proper type */
} scm_string;

/* scm_string_new doesn't copy buf */
scm_object *scm_string_new(char *buf, int len) {
    scm_string *str = malloc(sizeof(scm_string));

    str->base.type = scm_type_string;

    if (len < 0) {
        len = strlen(buf);
    }

    str->buf = buf;
    str->len = len;

    return (scm_object *)str;
}

/* scm_string_copy_new copies buf, for tests */
scm_object *scm_string_copy_new(const char *buf, int len) {
    if (len < 0) {
        len = strlen(buf);
    }

    char *new_buf = malloc(len + 1);
    strncpy(new_buf, buf, len);
    new_buf[len] = '\0';

    return scm_string_new(new_buf, len);
}

static void scm_string_free(scm_object *obj) {
    scm_string *s = (scm_string *)obj;
    if (s->buf) {
        free(s->buf);
    }
    free(s);
}

int scm_string_length(scm_object *obj) {
    scm_string *s = (scm_string *)obj;
    return s->len;
}

scm_object *scm_string_ref(scm_object *obj, int k) {
    scm_string *s = (scm_string *)obj;
    if (k < 0 || k >= s->len) {
        return NULL;
    }
    return (scm_object *)scm_chars[(int)s->buf[k]];
}

static int initialized = 0;
int scm_string_env_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_string, scm_string_free);

    initialized = 1;
    return 0;
}
