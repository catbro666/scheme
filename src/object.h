#ifndef SCHEME_OBJECT_H
#define SCHEME_OBJECT_H
#include <stdint.h>     /* intptr_t */

/* no data: eof, true, false, null
 * dot and rparen are only for internal use */
typedef enum {
    scm_type_eof = 0,
    scm_type_null,
    scm_type_dot,
    scm_type_rparen,
    scm_type_void,
    scm_type_true,
    scm_type_false,
    scm_type_char,
    scm_type_integer,
    scm_type_float,
    scm_type_string,
    scm_type_identifier,
    scm_type_pair,
    scm_type_vector,
    scm_type_input_port,
    scm_type_output_port,
    scm_type_primitive,
    scm_type_compound,
    scm_type_max,
} scm_type;

typedef struct scm_object_st {
    scm_type type;
} scm_object;

extern scm_object *scm_eof;
extern scm_object *scm_true;
extern scm_object *scm_false;
extern scm_object *scm_dot;
extern scm_object *scm_rparen;
extern scm_object *scm_null;
extern scm_object *scm_void;

/* free function of the specific type */
typedef void (*scm_object_free_fn)(scm_object *obj);
typedef int (*scm_eq_fn)(scm_object *o1, scm_object *o2);
typedef struct scm_object_methods_st {
    scm_object_free_fn free;
    scm_eq_fn eqv;
    scm_eq_fn equal;
} scm_object_methods;

scm_object *scm_boolean(int);
scm_object *scm_and(scm_object *b1, scm_object *b2);
scm_object *scm_or(scm_object *b1, scm_object *b2);
scm_object *scm_not(scm_object *b1);
extern scm_object_methods simple_methods;

void scm_object_free(scm_object *obj);
int scm_eq(scm_object *o1, scm_object *o2);
int scm_eqv(scm_object *o1, scm_object *o2);
int scm_equal(scm_object *o1, scm_object *o2);

/* predicate primitives */
extern scm_object *pred_null;
extern scm_object *pred_boolean;
extern scm_object *pred_char;
extern scm_object *pred_integer;
extern scm_object *pred_real;
extern scm_object *pred_number;
extern scm_object *pred_string;
extern scm_object *pred_symbol;
extern scm_object *pred_pair;
extern scm_object *pred_vector;
extern scm_object *pred_input_port;
extern scm_object *pred_output_port;
extern scm_object *pred_port;
extern scm_object *pred_procedure;

void scm_object_register(scm_type, scm_object_methods *methods);

int scm_object_init(void);
int scm_object_init_env(scm_object *env);

#endif /* SCHEME_OBJECT_H */

