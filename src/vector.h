#ifndef SCHEME_VECTOR_H
#define SCHEME_VECTOR_H
#include "object.h"

scm_object *scm_vector_new(int count, ...);
void scm_vector_fill(scm_object *obj, scm_object *fill);
scm_object *scm_vector_new_fill(int count, scm_object *fill);
scm_object *scm_vector_ref(scm_object *obj, int k);
int scm_vector_length(scm_object *obj);
int scm_vector_env_init(void);

#endif /* SCHEME_VECTOR_H */
