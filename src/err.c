#include "err.h"

#include <stdio.h>
#include <stdarg.h>

static const int msg_size = 1024;
static char msg[msg_size];

jmp_buf *scm_jmp = NULL;

void scm_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, msg_size, fmt, args);
    va_end(args);

    SCM_THROW;
}

void scm_error_free(scm_error_free_fn fn, void *p, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, msg_size, fmt, args);
    va_end(args);

    fn(p);
    SCM_THROW;
}

char *scm_error_msg() {
    return msg;
}

