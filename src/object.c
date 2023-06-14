#include "object.h"

#include "port.h"

#include <stdlib.h>

struct scm_object_st {
    scm_type type;
    intptr_t data;
};

static scm_object_data_free_fn frees[scm_type_max] = {0};

scm_object *scm_object_new(scm_type type, intptr_t data) {
    scm_object *obj = malloc(sizeof(scm_object));
    if (obj == NULL) return NULL;

    obj->type = type;
    obj->data = data;

    return obj;
}

void scm_object_free(scm_object *obj) {
    scm_object_data_free_fn data_free = frees[obj->type];
    if (data_free) {
        data_free((void *)obj->data);
    }

    free(obj);
}

scm_type scm_object_get_type(const scm_object *obj) {
    return obj->type;
}

intptr_t scm_object_get_data(const scm_object *obj) {
    return obj->data;
}

/* simple objects */

/* can't be free'ed, i.e. can't be sweeped by gc */
static scm_object scm_eof_arr[1] = {{scm_type_eof, (intptr_t)NULL}};
static scm_object scm_true_arr[1] = {{scm_type_true, (intptr_t)NULL}};
static scm_object scm_false_arr[1] = {{scm_type_false, (intptr_t)NULL}};
const int CHAR_NUM = 128;
static scm_object scm_chars_arr[CHAR_NUM];
scm_object *scm_chars[CHAR_NUM];
scm_object *scm_eof = scm_eof_arr;
scm_object *scm_true = scm_true_arr;
scm_object *scm_false = scm_false_arr;

static int initialized = 0;

int scm_object_env_init(void) {
    int i = 0;

    if (initialized)
        return 0;

    for (i = 0; i < CHAR_NUM; ++i) {
        scm_chars_arr[i].type = scm_type_char;
        scm_chars_arr[i].data = (intptr_t)i;
        scm_chars[i] = scm_chars_arr + i;
    }

    initialized = 1;
    return 0;
}

void scm_object_register(scm_type type, scm_object_data_free_fn fn) {
    frees[type] = fn;
    return;
}
