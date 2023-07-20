#ifndef SCHEME_STRING_H
#define SCHEME_STRING_H
#include "object.h"

scm_object *scm_string_new(char *str, long size);
scm_object *scm_string_copy_new(const char *buf, long len);
long  scm_string_length(scm_object *str);
scm_object *scm_string_ref(scm_object *str, long k);
char scm_string_get_char(scm_object *obj, long k);

int scm_string_init(void);
int scm_string_init_env(scm_object *env);

#endif /* SCHEME_STRING_H */

