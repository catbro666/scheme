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

/* Need to support nested extended symbol? I think not.
 * Because extended symbols are generated when expanding the template,
 * if an identifer in template is already a extended symbol, then it must
 * be in a nested syntax define, so the env doesn't change, the identifier
 * should refer to the same thing. */
typedef struct scm_extended_symbol_st {
    scm_object base;
    scm_object *sym;
    unsigned int uid;
    scm_object *env;
} scm_esymbol;

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

scm_object *scm_esymbol_new(scm_object *sym, unsigned int uid, scm_object *env) {
    scm_esymbol *esym = malloc(sizeof(scm_esymbol));

    esym->base.type = scm_type_eidentifier;
    esym->sym = sym;
    esym->uid = uid;
    esym->env = env;

    return (scm_object *)esym;
}

static void esymbol_free(scm_object *obj) {
    free(obj);
}

char *scm_esymbol_get_string(scm_object *obj) {
    scm_esymbol *s = (scm_esymbol *)obj;
    return scm_symbol_get_string(s->sym);
}

scm_object *scm_esymbol_get_symbol(scm_object *obj) {
    scm_esymbol *s = (scm_esymbol *)obj;
    return s->sym;
}

unsigned int scm_esymbol_get_uid(scm_object *obj) {
    scm_esymbol *s = (scm_esymbol *)obj;
    return s->uid;
}

scm_object *scm_esymbol_get_env(scm_object *obj) {
    scm_esymbol *s = (scm_esymbol *)obj;
    return s->env;
}

char *scm_variable_get_string(scm_object *obj) {
    if (IS_RAW_IDENTIFIER(obj))
        return scm_symbol_get_string(obj);
    else
        return scm_esymbol_get_string(obj);
}

static int esymbol_eqv(scm_object *o1, scm_object *o2) {
    scm_esymbol *s1 = (scm_esymbol *)o1;
    scm_esymbol *s2 = (scm_esymbol *)o2;
    return o1 == o2 || (symbol_eqv(o1, o2) && s1->uid == s2->uid);
}

static scm_object_methods symbol_methods = { symbol_free, symbol_eqv, symbol_eqv };
static scm_object_methods esymbol_methods = { esymbol_free, esymbol_eqv, esymbol_eqv };

static int initialized = 0;
int scm_symbol_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_identifier, &symbol_methods);
    scm_object_register(scm_type_eidentifier, &esymbol_methods);

    initialized = 1;
    return 0;
}

int scm_symbol_init_env(scm_object *env) {
    (void)env;
    return 0;
}
