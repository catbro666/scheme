#ifndef SCHEME_PROC_H
#define SCHEME_PROC_H
#include "object.h"
#include "pair.h"

/* signature of primitives */
typedef scm_object *(*prim_fn)(int n, scm_object *args);

scm_object *scm_primitive_new(const char *name, prim_fn fn, int min_arity,
                              int max_arity, scm_object *preds);
scm_object *scm_compound_new(scm_object *params, scm_object *body, scm_object *env);
scm_object *scm_apply(scm_object *opt, int n, scm_object *opds);

#define define_primitive_0(name) \
    static scm_object *prim_##name(int n, scm_object *args) { \
        (void)n; (void)args;\
        return scm_##name(); \
    }

#define define_primitive_1(name) \
    static scm_object *prim_##name(int n, scm_object *args) { \
        (void)n; \
        return scm_##name(scm_car(args)); \
    }

#define define_primitive_2(name) \
    static scm_object *prim_##name(int n, scm_object *args) { \
        (void)n; \
        return scm_##name(scm_car(args), scm_cadr(args)); \
    }

#define define_primitive_3(name) \
    static scm_object *prim_##name(int n, scm_object *args) { \
        (void)n; \
        return scm_##name(scm_car(args), scm_cadr(args), scm_caddr(args)); \
    }

#define define_primitive_0n(name) \
    static scm_object *prim_##name(int n, scm_object *args) { \
        (void)n;\
        return scm_##name(n, args); \
    }

#define define_primitive_1n(name) \
    static scm_object *prim_##name(int n, scm_object *args) { \
        (void)n; \
        return scm_##name(n, scm_car(args), scm_cdr(args)); \
    }

#define define_primitive_2n(name) \
    static scm_object *prim_##name(int n, scm_object *args) { \
        (void)n; \
        return scm_##name(n, scm_car(args), scm_cadr(args), scm_cddr(args)); \
    }

#define define_primitive_3n(name) \
    static scm_object *prim_##name(int n, scm_object *args) { \
        (void)n; \
        return scm_##name(n, scm_car(args), scm_cadr(args), scm_caddr(args), scm_cdddr(args)); \
    }


#endif /* SCHEME_PROC_H */

