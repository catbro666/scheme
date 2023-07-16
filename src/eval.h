#ifndef SCHEME_EVAL_H
#define SCHEME_EVAL_H
#include "object.h"

scm_object *scm_eval(scm_object *exp, scm_object *env);
scm_object *eval_sequence(scm_object *exp, scm_object *env);

#endif /* SCHEME_EVAL_H */

