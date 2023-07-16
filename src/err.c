#include "err.h"
#include "port.h"
#include "write.h"

#include <stdio.h>
#include <stdarg.h>

static const int msg_size = 4096;
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

void scm_error_object(scm_object *obj, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(msg, msg_size, fmt, args);
    va_end(args);

    scm_object *port = string_output_port_new(msg + n, msg_size - n - 1);
    int m = scm_write(port, obj);
    msg[n+m] = '\0';

    SCM_THROW;
}

char *scm_error_msg() {
    return msg;
}
