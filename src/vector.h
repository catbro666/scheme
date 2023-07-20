#ifndef SCHEME_VECTOR_H
#define SCHEME_VECTOR_H
#include "object.h"

extern scm_object *scm_empty_vector;

scm_object *scm_vector_alloc(long count);
scm_object *scm_vector_realloc(scm_object *vector, long count);
scm_object *scm_vector_new(long count, ...);
void scm_vector_fill(scm_object *vector, scm_object *fill);
scm_object *scm_vector_new_fill(long count, scm_object *fill);
scm_object *scm_vector_ref(scm_object *vector, long k);
scm_object *scm_vector_set(scm_object *vector, long k, scm_object *obj);
long scm_vector_length(scm_object *vector);
scm_object *scm_vector_insert(scm_object *vector, scm_object *obj);
int scm_vector_init(void);
int scm_vector_init_env(scm_object *env);

#endif /* SCHEME_VECTOR_H */

