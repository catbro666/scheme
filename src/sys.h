#ifndef SCHEME_SYS_H
#define SCHEME_SYS_H
#include "err.h"

void scm_sys_err(const char *fmt, ...);
void scm_sys_err_free(scm_error_free_fn fn, void *p, const char *fmt, ...);

#endif /* SCHEME_SYS_H */

