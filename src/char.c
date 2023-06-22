#include "char.h"

typedef struct scm_char_st {
    scm_object base;
    char c;
} scm_char;

static const int CHAR_NUM = 128;
static scm_char scm_chars_arr[CHAR_NUM];
scm_object *scm_chars[CHAR_NUM];

char scm_char_get_char(scm_object *obj) {
    scm_char *c = (scm_char *)obj;
    return c->c;
}

static int initialized = 0;

int scm_char_env_init(void) {
    int i = 0;

    if (initialized)
        return 0;

    for (i = 0; i < CHAR_NUM; ++i) {
        scm_chars_arr[i].base.type = scm_type_char;
        scm_chars_arr[i].c = (char)i;
        scm_chars[i] = (scm_object *)(scm_chars_arr + i);
    }

    initialized = 1;
    return 0;
}
