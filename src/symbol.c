#include "symbol.h"
#include "proc.h"
#include "env.h"

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

static void symbol_free(scm_object *obj) {
    /* no need to free symbols after we implement hash table */
    (void)obj;
    return;
}

char *scm_symbol_get_string(scm_object *obj) {
    scm_symbol *s = (scm_symbol *)obj;
    return s->buf;
}

/* comparing pointer only is enough after we use hash table */
static int symbol_eqv(scm_object *o1, scm_object *o2) {
    scm_symbol *s1 = (scm_symbol *)o1;
    scm_symbol *s2 = (scm_symbol *)o2;
    return o1 == o2 || (s1->len == s2->len && !strncmp(s1->buf, s2->buf, s1->len));
}

static scm_object_methods symbol_methods = { symbol_free, symbol_eqv, symbol_eqv };

static int initialized = 0;
int scm_symbol_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_identifier, &symbol_methods);

    initialized = 1;
    return 0;
}

int scm_symbol_init_env(scm_object *env) {
    (void)env;
    return 0;
}
