#ifndef SCHEME_XFORM_H
#define SCHEME_XFORM_H
#include "object.h"

scm_object *scm_transformer_new(scm_object *kw, scm_object *literals, scm_object *rules, scm_object *env);
void check_syntax_rule(scm_object *rule, scm_object *literals);
scm_object *transform_macro(scm_object *xformer, scm_object *form, scm_object *env);

extern int ellipsis_duplicate_mode;
int scm_xform_init(void);

/* test purpose */
long pid_binding_get_depth(scm_object *b);
scm_object *pid_binding_get_form(scm_object *b);
scm_object *pid_binding_get_vect(scm_object *b);
scm_object *pid_binding_get_binding(scm_object *b);
scm_object *lookup_pid_binding(scm_object *pids, scm_object *pid);

scm_object *check_pattern(scm_object *pat, scm_object *literals);
int check_template(scm_object *temp, scm_object *pids);
scm_object *match_pattern(scm_object *lf, scm_object *lp,
                          scm_object *literals, scm_object *oenv, scm_object *nenv);
scm_object *expand_template(scm_object *temp, scm_object *pids, scm_object *env);

#endif /* SCHEME_XFORM_H */

