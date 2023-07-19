#include "err.h"
#include "port.h"
#include "write.h"

#include <stdio.h>

static const int msg_size = 4096;
static char msg[msg_size];
static int pos = 0;

jmp_buf *scm_jmp = NULL;

void scm_error_add(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    pos += vsnprintf(msg + pos, msg_size - pos, fmt, args);
    va_end(args);
}

void scm_error_vadd(const char *fmt, va_list args) {
    pos += vsnprintf(msg + pos, msg_size - pos, fmt, args);
}

void scm_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg + pos, msg_size - pos, fmt, args);
    va_end(args);

    pos = 0;
    SCM_THROW;
}

void scm_error_free(scm_error_free_fn fn, void *p, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg + pos, msg_size - pos, fmt, args);
    va_end(args);

    fn(p);
    pos = 0;
    SCM_THROW;
}

void scm_error_object(scm_object *obj, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    pos += vsnprintf(msg + pos, msg_size - pos, fmt, args);
    va_end(args);

    scm_object *port = string_output_port_new(msg + pos, msg_size - pos - 1);
    int m = scm_write(port, obj);
    msg[pos+m] = '\0';

    pos = 0;
    SCM_THROW;
}

char *scm_error_msg() {
    return msg;
}
