#ifndef SCHEME_NUMBER_H
#define SCHEME_NUMBER_H
#include "object.h"

scm_object *scm_number_new_integer(const char *num, int radix);
scm_object *scm_number_new_integer_from_float(const char *num);
scm_object *scm_number_new_float(const char *num);
scm_object *scm_number_new_float_from_integer(const char *num, int radix);
scm_object *INTEGER(int n);
scm_object *FLOAT(double n);

char *scm_number_to_string(scm_object *obj, int radix);
int scm_integer_get_val(scm_object *obj);
double scm_float_get_val(scm_object *obj);

int scm_number_init(void);
int scm_number_init_env(scm_object *env);

#endif /* SCHEME_NUMBER_H */

