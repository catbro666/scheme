#include "number.h"

#include <errno.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* complex = 2 real objects combination
 * real = integer | rational | float
 * exact   : integer / rational
 * inexact : float
 * rational = 2 integer objects combination
 * exact order  : integer -> rational -> complex, can auto-degrade
 * inexact order: float -> complex, can't auto-degrade
 * exact -> inexact */

typedef struct scm_integer_st {
    scm_object base;
    long val;
} scm_integer;

typedef struct scm_float_st {
    scm_object base;
    double val;
} scm_float;

static const char chars[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static void number_free(scm_object *obj) {
    free(obj);
}

scm_object *scm_number_new_integer(const char *num, int radix) {
    scm_integer *i = malloc(sizeof(scm_integer));

    i->base.type = scm_type_integer;

    errno = 0;
    i->val = strtol(num, NULL, radix);
    if (errno == ERANGE) {  /* TODO: support bn */
        goto err;
    }

    return (scm_object *)i;
err:
    free(i);
    return NULL;
}

scm_object *scm_number_new_float_from_integer(const char *num, int radix) {
    long val;
    scm_float *f = malloc(sizeof(scm_float));

    f->base.type = scm_type_float;

    errno = 0;
    val = strtol(num, NULL, radix);
    if (errno == ERANGE) {  /* TODO: support bn */
        goto err;
    }
    f->val = (double)val;   /* XXX: lost precision */

    return (scm_object *)f;
err:
    free(f);
    return NULL;
}

scm_object *scm_number_new_float(const char *num) {
    scm_float *f = malloc(sizeof(scm_float));

    f->base.type = scm_type_float;

    errno = 0;
    f->val = strtod(num, NULL);
    if (errno == ERANGE) {
        goto err;
    }

    return (scm_object *)f;
err:
    free(f);
    return NULL;
}

scm_object *scm_number_new_integer_from_float(const char *num) {
    double val;
    scm_integer *i = malloc(sizeof(scm_integer));

    i->base.type = scm_type_integer;

    errno = 0;
    val = strtod(num, NULL);
    if (errno == ERANGE) {
        goto err;
    }
    i->val = lrint(val);  /* XXX: lost precision */

    return (scm_object *)i;
err:
    free(i);
    return NULL;
}

int scm_number_is_exact(scm_object *obj) {
    if (obj->type == scm_type_integer) {
        return 1;
    }
    else {
        return 0;
    }
}

static int log2n(int x) {
    int result = -1;
    while (x > 0) {
        x >>= 1;
        result++;
    }
    return result;
}

static void ulong_to_string(unsigned long val, char *buf, int radix) {
    int bits_per_digit = log2n(radix);
    int bits = sizeof(unsigned long) * 8;
    int r = bits % bits_per_digit;
    r = r ? r : bits_per_digit;
    int has_digit = 0;
    unsigned long mask = radix - 1;

    for (bits -= r; bits >= 0; bits = bits - bits_per_digit) {
        int c = (val >> bits) & mask;
        if (c || has_digit) {
            *buf++ = chars[c];
            has_digit = 1;
        }
    }
    assert(bits == 0 - bits_per_digit);

    if (!has_digit) {
        *buf++ = '0';
    }
    *buf = '\0';
}

char *scm_number_to_string(scm_object *obj, int radix) {
    char *buf, *s;
    buf = malloc(256+2);
    s = buf;

    if (obj->type == scm_type_integer) {
        scm_integer *i = (scm_integer *)obj;
        unsigned long val;
        switch (radix) {
        case 16:
        case 8:
        case 2:
            if (i->val < 0) {
                buf[0] = '-';
                ++s;
                val = -i->val;
            }
            else {
                val = i->val;
            }
            ulong_to_string(val, s, radix);
            break;
        default: /* 10 */
            (void) snprintf(buf, 256, "%ld", i->val);
            break;
        }
    }
    else {
        if (radix != 10) {
            free(buf);
            return NULL;    /* inexact number only support radix 10 */
        }
        scm_float *f = (scm_float *)obj;
        (void) snprintf(buf, 256, "%.16g", f->val);
        /* add .0 if it looks like an integer */
        while (*s != 'E' && *s != 'e' && *s != '.') {
            if (*s++ == 0) {
                sprintf(s-1, ".0");
                break;
            }
        }
    }

    return buf;
}

static int integer_eqv(scm_object *o1, scm_object *o2) {
    return ((scm_integer *)o1)->val == ((scm_integer *)o2)->val;
}

static int float_eqv(scm_object *o1, scm_object *o2) {
    return ((scm_float *)o1)->val == ((scm_float *)o2)->val;
}

static scm_object_methods integer_methods = { number_free, integer_eqv, integer_eqv };
static scm_object_methods float_methods = { number_free, float_eqv, float_eqv };

static int initialized = 0;

int scm_number_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_integer, &integer_methods);
    scm_object_register(scm_type_float, &float_methods);

    initialized = 1;
    return 0;
}

