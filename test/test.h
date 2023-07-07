#ifndef SCHEME_TEST_H
#define SCHEME_TEST_H

#include "../src/object.h"
#include "../src/port.h"
#include "../src/err.h"
#include "../src/char.h"
#include "../src/number.h"
#include "../src/string.h"
#include "../src/symbol.h"
#include "../src/token.h"
#include "../src/pair.h"
#include "../src/vector.h"
#include "../src/read.h"
#include "../src/env.h"

#include <tau/tau.h>

/* @msg: the expected error msg
 * @exp: the expression that is expected to throw exception */
#define REQUIRE_EXC(msg, exp) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        REQUIRE_SUBSTREQ(err, msg, strlen(msg)); \
    } while (0)

#define REQUIRE_NOEXC(exp) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        REQUIRE(!err, "no exception throwed"); \
    } while (0)

#define REQUIRE_NOEXC_EQ(exp, val) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            REQUIRE_EQ(exp, val); \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        REQUIRE(!err, "no exception throwed"); \
    } while (0)

#define CHECK_EXC(msg, exp) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        CHECK_SUBSTREQ(err, msg, strlen(msg)); \
    } while (0)

#define CHECK_NOEXC(exp) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        CHECK(!err, "no exception throwed"); \
    } while (0);

#define CHECK_NOEXC_EQ(exp, val) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            CHECK_EQ(exp, val); \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        CHECK(!err, "no exception throwed"); \
    } while (0)

#endif /* SCHEME_TEST_H */

