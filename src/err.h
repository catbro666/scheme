#ifndef SCHEME_ERR_H
#define SCHEME_ERR_H
#include <setjmp.h>

extern jmp_buf *scm_jmp;

#define SCM_TRY do { \
    jmp_buf *prev_jmp = scm_jmp; \
    jmp_buf curr_jmp; \
    scm_jmp = &curr_jmp; \
    if (setjmp(curr_jmp) == 0)

#define SCM_CATCH \
    else { \
        scm_jmp = prev_jmp;

#define SCM_END_TRY \
    } \
    scm_jmp = prev_jmp; \
} while (0)

#define SCM_THROW \
    longjmp(*scm_jmp, 1)

typedef void (*scm_error_free_fn) (void *);
void scm_error(const char *fmt, ...);
/* @fn: function pointer used to release resource before throw */
void scm_error_free(scm_error_free_fn fn, void *p, const char *fmt, ...);
char *scm_error_msg();

#endif /* SCHEME_ERR_H */

