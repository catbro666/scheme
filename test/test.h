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
#include "../src/write.h"
#include "../src/env.h"

#include <tau/tau.h>
#include <limits.h>

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

#define TEST_INIT() \
    do { \
        int res; \
        res = scm_object_init(); \
        REQUIRE(!res, "scm_object_init"); \
        res = scm_char_init(); \
        REQUIRE(!res, "scm_char_init"); \
        res = scm_port_init(); \
        REQUIRE(!res, "scm_port_init"); \
        res = scm_string_init(); \
        REQUIRE(!res, "scm_string_init"); \
        res = scm_symbol_init(); \
        REQUIRE(!res, "scm_symbol_init"); \
        res = scm_number_init(); \
        REQUIRE(!res, "scm_number_init"); \
        res = scm_token_init(); \
        REQUIRE(!res, "scm_token_init"); \
        res = scm_pair_init(); \
        REQUIRE(!res, "scm_pair_init"); \
        res = scm_vector_init(); \
        REQUIRE(!res, "scm_vector_init"); \
    } while (0)

#endif /* SCHEME_TEST_H */

