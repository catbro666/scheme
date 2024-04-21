#include "sys.h"

#include <string.h>
#include <errno.h>
#include <stdarg.h>

void scm_sys_err(const char *fmt, ...) {
    int eno = errno;
    va_list args;
    va_start(args, fmt);
    scm_error_vadd(fmt, args);
    va_end(args);

    scm_error("system error: %s, errno=%d", strerror(eno), eno);
}
