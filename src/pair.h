#ifndef SCHEME_PAIR_H
#define SCHEME_PAIR_H
#include "object.h"

extern scm_object *scm_null;

scm_object *scm_cons(scm_object *car, scm_object *cdr);
scm_object *scm_car(scm_object *pair);
scm_object *scm_cdr(scm_object *pair);
void scm_set_car(scm_object *pair, scm_object *o);
void scm_set_cdr(scm_object *pair, scm_object *o);
scm_object *scm_list(int count, ...);
scm_object *scm_list_ref(scm_object *list, int k);
int scm_list_length(scm_object *list);

int scm_pair_env_init(void);

#endif /* SCHEME_PAIR_H */

