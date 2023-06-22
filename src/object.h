#ifndef SCHEME_OBJECT_H
#define SCHEME_OBJECT_H
#include <stdint.h>     /* intptr_t */

/* no data: scm_type_eof, scm_type_true, scm_type_false */
/* int data: scm_type_char, scm_type_integer */
typedef enum {
    scm_type_eof = 0,
    scm_type_null,
    scm_type_true,
    scm_type_false,
    scm_type_char,
    scm_type_integer,
    scm_type_float,
    scm_type_string,
    scm_type_identifier,
    scm_type_symbol,
    scm_type_pair,
    scm_type_vector,
    scm_type_input_port,
    scm_type_output_port,
    scm_type_max,
} scm_type;

typedef struct scm_object_st {
    scm_type type;
} scm_object;

extern scm_object *scm_eof;
extern scm_object *scm_true;
extern scm_object *scm_false;

/* free function of the specific type */
typedef void (*scm_object_data_free_fn)(scm_object *data);

void scm_object_free(scm_object *obj);

void scm_object_register(scm_type, scm_object_data_free_fn fn);

int scm_object_env_init(void);


#endif /* SCHEME_OBJECT_H */

