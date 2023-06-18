#include "string.h"
#include <stdlib.h>
#include <string.h>

typedef struct scm_string_st {
    char *buf;
    int len;   /* NOTE: need proper type */
} scm_string;

scm_object *scm_string_new(char *buf, int len) {
    scm_string *str = malloc(sizeof(scm_string));

    if (len < 0) {
        len = strlen(buf);
    }

    str->buf = buf;
    str->len = len;

    scm_object *obj = scm_object_new(scm_type_string, (intptr_t)str);
    if (!obj) {
        if (str->buf) {
            free(str->buf);
        }
        free(str);
    }

    return obj;
}

static void scm_string_free(void *str) {
    scm_string *s = (scm_string *)str;
    if (s->buf) {
        free(s->buf);
    }
    free(s);
}

int scm_string_length(scm_object *str) {
    scm_string *data = (scm_string *)scm_object_get_data(str);
    return data->len;
}

scm_object *scm_string_ref(scm_object *str, int k) {
    scm_string *data = (scm_string *)scm_object_get_data(str);
    if (k < 0 || k >= data->len) {
        return NULL;
    }
    return scm_chars[(int)data->buf[k]];
}

static int initialized = 0;
int scm_string_env_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_string, scm_string_free);

    initialized = 1;
    return 0;
}
