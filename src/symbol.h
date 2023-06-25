#ifndef SCHEME_SYMBOL_H
#define SCHEME_SYMBOL_H
#include "object.h"

scm_object *scm_symbol_new(char *str, int size);

char *scm_symbol_get_string(scm_object *obj);

int scm_symbol_env_init(void);
#endif /* SCHEME_SYMBOL_H */

