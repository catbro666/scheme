#ifndef SCHEME_EVAL_H
#define SCHEME_EVAL_H
#include "object.h"

scm_object *scm_eval(scm_object *exp, scm_object *env);
scm_object *scm_eval_sequence(scm_object *exp, scm_object *env);
scm_object *scm_apply(scm_object *opt, int n, scm_object *opds);

int scm_eval_init(void);
int scm_eval_init_env(scm_object *env);
#endif /* SCHEME_EVAL_H */

