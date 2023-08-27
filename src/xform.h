#ifndef SCHEME_XFORM_H
#define SCHEME_XFORM_H
#include "object.h"

scm_object *scm_transformer_new(scm_object *kw, scm_object *literals, scm_object *rules, scm_object *env);
void check_syntax_rule(scm_object *rule, scm_object *literals);
scm_object *transform_macro(scm_object *xformer, scm_object *form, scm_object *env);

int scm_xform_init(void);
#endif /* SCHEME_XFORM_H */

