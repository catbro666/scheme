#ifndef SCHEME_PAIR_H
#define SCHEME_PAIR_H
#include "object.h"

scm_object *scm_cons(scm_object *car, scm_object *cdr);
scm_object *scm_car(scm_object *pair);
scm_object *scm_cdr(scm_object *pair);
scm_object *scm_set_car(scm_object *pair, scm_object *o);
scm_object *scm_set_cdr(scm_object *pair, scm_object *o);
scm_object *scm_list(long count, ...);
scm_object *scm_list_ref(scm_object *list, long k);
scm_object *scm_list_combine(scm_object *l1, scm_object *l2);
long scm_list_quasilength(scm_object *list);
long scm_list_length(scm_object *list);
scm_object *scm_list_last_pair(scm_object *l);
/* the caller should ensure `l` a proper list */
int scm_memq(scm_object *o, scm_object *l);
int scm_memv(scm_object *o, scm_object *l);
int scm_member(scm_object *o, scm_object *l);
scm_object *scm_assq(scm_object *o, scm_object *l);
scm_object *scm_assv(scm_object *o, scm_object *l);
scm_object *scm_assoc(scm_object *o, scm_object *l);

scm_object *scm_caar(scm_object *pair);
scm_object *scm_cadr(scm_object *pair);
scm_object *scm_cdar(scm_object *pair);
scm_object *scm_cddr(scm_object *pair);
scm_object *scm_caaar(scm_object *pair);
scm_object *scm_caadr(scm_object *pair);
scm_object *scm_cadar(scm_object *pair);
scm_object *scm_caddr(scm_object *pair);
scm_object *scm_cdaar(scm_object *pair);
scm_object *scm_cdadr(scm_object *pair);
scm_object *scm_cddar(scm_object *pair);
scm_object *scm_cdddr(scm_object *pair);
scm_object *scm_caaaar(scm_object *pair);
scm_object *scm_caaadr(scm_object *pair);
scm_object *scm_caadar(scm_object *pair);
scm_object *scm_caaddr(scm_object *pair);
scm_object *scm_cadaar(scm_object *pair);
scm_object *scm_cadadr(scm_object *pair);
scm_object *scm_caddar(scm_object *pair);
scm_object *scm_cadddr(scm_object *pair);
scm_object *scm_cdaaar(scm_object *pair);
scm_object *scm_cdaadr(scm_object *pair);
scm_object *scm_cdadar(scm_object *pair);
scm_object *scm_cdaddr(scm_object *pair);
scm_object *scm_cddaar(scm_object *pair);
scm_object *scm_cddadr(scm_object *pair);
scm_object *scm_cdddar(scm_object *pair);
scm_object *scm_cddddr(scm_object *pair);

#define FOREACH_LIST(o, l) \
    for (scm_object *o; (l->type == scm_type_pair) && (o = scm_car(l)) && (l = scm_cdr(l));)

int scm_pair_init(void);
int scm_pair_init_env(scm_object *env);

#endif /* SCHEME_PAIR_H */

