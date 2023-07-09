#ifndef SCHEME_CHAR_H
#define SCHEME_CHAR_H
#include "object.h"

extern scm_object *scm_chars[];

char scm_char_get_char(scm_object *obj);

int scm_char_init(void);

#endif /* SCHEME_CHAR_H */

