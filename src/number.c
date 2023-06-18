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
    long val;
} scm_integer;

typedef struct scm_float_st {
    double val;
} scm_float;

static const char chars[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static void scm_number_free(void *n) {
    free(n);
}

scm_object *scm_number_new_integer(const char *num, int radix) {
    scm_object *obj = NULL;
    scm_integer *data = malloc(sizeof(scm_integer));

    errno = 0;
    data->val = strtol(num, NULL, radix);
    if (errno == ERANGE) {  /* TODO: support bn */
        goto err;
    }

    obj = scm_object_new(scm_type_integer, (intptr_t)data);

err:
    if (!obj) {
        if (data) free(data);
    }
    return obj;
}

scm_object *scm_number_new_float_from_integer(const char *num, int radix) {
    scm_object *obj = NULL;
    scm_float *data = malloc(sizeof(scm_float));
    long val;

    errno = 0;
    val = strtol(num, NULL, radix);
    if (errno == ERANGE) {  /* TODO: support bn */
        goto err;
    }
    data->val = (double)val;   /* XXX: lost precision */

    obj = scm_object_new(scm_type_float, (intptr_t)data);

err:
    if (!obj) {
        if (data) free(data);
    }
    return obj;
}

scm_object *scm_number_new_float(const char *num) {
    scm_object *obj = NULL;
    scm_float *data = malloc(sizeof(scm_float));

    errno = 0;
    data->val = strtod(num, NULL);
    if (errno == ERANGE) {
        goto err;
    }

    obj = scm_object_new(scm_type_float, (intptr_t)data);

err:
    if (!obj) {
        if (data) free(data);
    }
    return obj;
}

scm_object *scm_number_new_integer_from_float(const char *num) {
    scm_object *obj = NULL;
    scm_integer *data = malloc(sizeof(scm_integer));
    double val;

    errno = 0;
    val = strtod(num, NULL);
    if (errno == ERANGE) {
        goto err;
    }
    data->val = lrint(val);  /* XXX: lost precision */

    obj = scm_object_new(scm_type_integer, (intptr_t)data);

err:
    if (!obj) {
        if (data) free(data);
    }
    return obj;
}

int scm_number_is_exact(scm_object *obj) {
    scm_type type = scm_object_get_type(obj);
    if (type == scm_type_integer) {
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

    scm_type type = scm_object_get_type(obj);
    if (type == scm_type_integer) {
        scm_integer *data = (scm_integer *)scm_object_get_data(obj);
        unsigned long val;
        switch (radix) {
        case 16:
        case 8:
        case 2:
            if (data->val < 0) {
                buf[0] = '-';
                ++s;
                val = -data->val;
            }
            else {
                val = data->val;
            }
            ulong_to_string(val, s, radix);
            break;
        default: /* 10 */
            (void) snprintf(buf, 256, "%ld", data->val);
            break;
        }
    }
    else {
        if (radix != 10) {
            free(buf);
            return NULL;    /* inexact number only support radix 10 */
        }
        scm_float *data = (scm_float *)scm_object_get_data(obj);
        (void) snprintf(buf, 256, "%.16g", data->val);
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

static int initialized = 0;

int scm_number_env_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_integer, scm_number_free);
    scm_object_register(scm_type_float, scm_number_free);

    initialized = 1;
    return 0;
}

