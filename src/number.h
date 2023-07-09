#ifndef SCHEME_NUMBER_H
#define SCHEME_NUMBER_H
#include "object.h"

scm_object *scm_number_new_integer(const char *num, int radix);
scm_object *scm_number_new_integer_from_float(const char *num);
scm_object *scm_number_new_float(const char *num);
scm_object *scm_number_new_float_from_integer(const char *num, int radix);

char *scm_number_to_string(scm_object *obj, int radix);
int scm_number_init(void);

#endif /* SCHEME_NUMBER_H */

