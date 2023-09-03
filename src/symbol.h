#ifndef SCHEME_SYMBOL_H
#define SCHEME_SYMBOL_H
#include "object.h"

scm_object *scm_symbol_new(const char *str, int size);
#define SYM(name) scm_symbol_new(#name, -1)
char *scm_symbol_get_string(scm_object *obj);

scm_object *scm_esymbol_new(scm_object *sym, unsigned int uid, scm_object *env);
scm_object *scm_esymbol_get_symbol(scm_object *obj);
char *scm_esymbol_get_string(scm_object *obj);
unsigned int scm_esymbol_get_uid(scm_object *obj);
scm_object *scm_esymbol_get_env(scm_object *obj);
char *scm_variable_get_string(scm_object *obj);
int same_id(scm_object *o1, scm_object *o2);

int scm_symbol_init(void);
int scm_symbol_init_env(scm_object *env);
#endif /* SCHEME_SYMBOL_H */

