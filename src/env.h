#ifndef SCHEME_ENV_H
#define SCHEME_ENV_H
#include "object.h"
#include "proc.h"

scm_object *scm_env_new();
scm_object *scm_env_extend(scm_object *env, scm_object *vars, scm_object *vals);
scm_object *scm_env_lookup_var(scm_object *env, scm_object *var);
int scm_env_set_var(scm_object *env, scm_object *var, scm_object *val);
void scm_env_define_var(scm_object *env, scm_object *var, scm_object *val);
int scm_env_add_prim(scm_object *env, const char *name, prim_fn fn,
                     int min_arity, int max_arity, scm_object *preds);
scm_object *scm_global_env();

#endif /* SCHEME_ENV_H */

