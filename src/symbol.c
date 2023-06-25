#include "symbol.h"
#include <stdlib.h>
#include <string.h>

/* TODO: support hash */
typedef struct scm_symbol_st {
    scm_object base;
    char *buf;
    int len;
} scm_symbol;

scm_object *scm_symbol_new(char *buf, int len) {
    scm_symbol *sym = malloc(sizeof(scm_symbol));

    sym->base.type = scm_type_symbol;

    if (len < 0) {
        len = strlen(buf);
    }

    sym->buf = buf;
    sym->len = len;

    return (scm_object *)sym;
}

static void scm_symbol_free(scm_object *obj) {
    scm_symbol *s = (scm_symbol *)obj;
    if (s->buf) {
        free(s->buf);
    }
    free(s);
}

char *scm_symbol_get_string(scm_object *obj) {
    scm_symbol *s = (scm_symbol *)obj;
    return s->buf;
}

static int initialized = 0;
int scm_symbol_env_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_symbol, scm_symbol_free);

    initialized = 1;
    return 0;
}
