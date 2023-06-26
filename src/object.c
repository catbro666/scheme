#include "object.h"

#include "port.h"

#include <stdlib.h>

static scm_object_data_free_fn frees[scm_type_max] = {0};

void scm_object_free(scm_object *obj) {
    scm_object_data_free_fn data_free = frees[obj->type];
    if (data_free) {
        data_free(obj);
    }
}

/* simple objects */

/* can't be free'ed, i.e. can't be sweeped by gc */
static scm_object scm_eof_arr[1] = {{scm_type_eof}};
static scm_object scm_true_arr[1] = {{scm_type_true}};
static scm_object scm_false_arr[1] = {{scm_type_false}};
static scm_object scm_dot_arr[1] = {{scm_type_dot}};
static scm_object scm_rparen_arr[1] = {{scm_type_rparen}};
scm_object *scm_eof = scm_eof_arr;
scm_object *scm_true = scm_true_arr;
scm_object *scm_false = scm_false_arr;
scm_object *scm_dot = scm_dot_arr;
scm_object *scm_rparen = scm_rparen_arr;

static int initialized = 0;

int scm_object_env_init(void) {
    if (initialized)
        return 0;

    initialized = 1;
    return 0;
}

void scm_object_register(scm_type type, scm_object_data_free_fn fn) {
    frees[type] = fn;
}
