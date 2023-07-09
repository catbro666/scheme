#ifndef SCHEME_SYMBOL_H
#define SCHEME_SYMBOL_H
#include "object.h"

scm_object *scm_symbol_new(const char *str, int size);

char *scm_symbol_get_string(scm_object *obj);

int scm_symbol_equal(scm_object *obj1, scm_object *obj2);

int scm_symbol_init(void);
#endif /* SCHEME_SYMBOL_H */

