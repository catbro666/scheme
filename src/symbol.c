#include "symbol.h"
#include <stdlib.h>
#include <string.h>

/* TODO: support hash */
typedef struct scm_symbol_st {
    scm_object base;
    char *buf;
    int len;
} scm_symbol;

scm_object *scm_symbol_new(const char *buf, int len) {
    scm_symbol *sym = malloc(sizeof(scm_symbol));

    sym->base.type = scm_type_identifier;

    if (len < 0) {
        len = strlen(buf);
    }

    sym->buf = malloc(len + 1);
    strncpy(sym->buf, buf, len);
    sym->buf[len] = '\0';
    sym->len = len;

    return (scm_object *)sym;
}

static void scm_symbol_free(scm_object *obj) {
    /* no need to free symbols after we implement hash table */
    (void)obj;
    return;
}

char *scm_symbol_get_string(scm_object *obj) {
    scm_symbol *s = (scm_symbol *)obj;
    return s->buf;
}

int scm_symbol_equal(scm_object *obj1, scm_object *obj2) {
    scm_symbol *s1 = (scm_symbol *)obj1;
    scm_symbol *s2 = (scm_symbol *)obj2;
    return s1->len == s2->len && !strncmp(s1->buf, s2->buf, s1->len);
}

static int initialized = 0;
int scm_symbol_env_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_identifier, scm_symbol_free);

    initialized = 1;
    return 0;
}
