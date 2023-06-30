#ifndef SCHEME_STRING_H
#define SCHEME_STRING_H
#include "object.h"

scm_object *scm_string_new(char *str, int size);
scm_object *scm_string_copy_new(const char *buf, int len);
int scm_string_length(scm_object *str);
scm_object *scm_string_ref(scm_object *str, int k);

int scm_string_env_init(void);

#endif /* SCHEME_STRING_H */

