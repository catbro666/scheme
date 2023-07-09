#include "object.h"

#include "port.h"

#include <stdlib.h>

static scm_object_methods *all_methods[scm_type_max] = {0};

void scm_object_free(scm_object *obj) {
    scm_object_methods *methods = all_methods[obj->type];
    if (methods && methods->free)
        methods->free(obj);
}

int scm_object_eq(scm_object *o1, scm_object *o2) {
    return scm_object_eqv(o1, o2);
}

int scm_object_eqv(scm_object *o1, scm_object *o2) {
    if (o1->type != o2->type)
        return 0;
    return all_methods[o1->type]->eqv(o1, o2);
}

int scm_object_equal(scm_object *o1, scm_object *o2) {
    if (o1->type != o2->type)
        return 0;
    return all_methods[o1->type]->equal(o1, o2);
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

static int always_eqv(scm_object *o1, scm_object *o2) {
    (void)o1; (void)o2;
    return 1;
}

scm_object_methods simple_methods = { NULL, always_eqv, always_eqv};

static int initialized = 0;

int scm_object_init(void) {
    if (initialized)
        return 0;

    scm_object_register(scm_type_eof, &simple_methods);
    scm_object_register(scm_type_true, &simple_methods);
    scm_object_register(scm_type_false, &simple_methods);

    initialized = 1;
    return 0;
}

void scm_object_register(scm_type type, scm_object_methods *methods) {
    all_methods[type] = methods;
}
