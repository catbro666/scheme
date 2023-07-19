#include "sys.h"

#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

char *sys_realpath(const char *path) {
    return realpath(path, NULL);
}

int sys_file_exists(const char *path) {
    return access(path, F_OK) == 0;
}
