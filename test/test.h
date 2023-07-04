#ifndef SCHEME_TEST_H
#define SCHEME_TEST_H

#include "../src/err.h"
#include "../src/object.h"
#include "../src/char.h"
#include "../src/number.h"
#include "../src/string.h"
#include "../src/symbol.h"
#include "../src/port.h"
#include "../src/token.h"
#include "../src/pair.h"
#include "../src/vector.h"
#include "../src/exp.h"
#include "../src/read.h"
#include "../src/env.h"
#include "../src/eval.h"

#include <tau/tau.h>

/* @msg: the expected error msg
 * @exp: the expression that is expected to throw exception */
#define REQUIRE_EXCEPTION(msg, exp) \
    char *err = NULL; \
    SCM_TRY { \
        exp; \
    } SCM_CATCH { \
        err = scm_error_msg(); \
    } SCM_END_TRY; \
 \
    REQUIRE_SUBSTREQ(err, msg, strlen(msg))

#define REQUIRE_NOEXCEPTION(exp) \
    char *err = NULL; \
    SCM_TRY { \
        exp; \
    } SCM_CATCH { \
        err = scm_error_msg(); \
    } SCM_END_TRY; \
 \
    REQUIRE(!err, "no exception throwed")

#define CHECK_EXCEPTION(msg, exp) \
    char *err = NULL; \
    SCM_TRY { \
        exp; \
    } SCM_CATCH { \
        err = scm_error_msg(); \
    } SCM_END_TRY; \
 \
    CHECK_SUBSTREQ(err, msg, strlen(msg))

#define CHECK_NOEXCEPTION(exp) \
    char *err = NULL; \
    SCM_TRY { \
        exp; \
    } SCM_CATCH { \
        err = scm_error_msg(); \
    } SCM_END_TRY; \
 \
    CHECK(!err, "no exception throwed")

#endif /* SCHEME_TEST_H */

