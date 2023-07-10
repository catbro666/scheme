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
#define REQUIRE_EXC_ORIGIN(msg, exp, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        REQUIRE_SUBSTREQ(err, msg, strlen(msg), __VA_ARGS__); \
    } while (0)

#define REQUIRE_EXC(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_EXC, __VA_ARGS__)

#define REQUIRE_NOEXC_ORIGIN(exp, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        REQUIRE(!err, __VA_ARGS__); \
    } while (0)

#define REQUIRE_NOEXC(...) FIXED1_CHOOSER(__VA_ARGS__)(REQUIRE_NOEXC, __VA_ARGS__)

#define REQUIRE_NOEXC_EQ_ORIGIN(exp, val, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            REQUIRE_EQ(exp, val, __VA_ARGS__); \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        REQUIRE(!err, __VA_ARGS__); \
    } while (0)

#define REQUIRE_NOEXC_EQ(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_NOEXC_EQ, __VA_ARGS__)

#define REQUIRE_OBJ_EQ_ORIGIN(actual, expected, ...) \
    REQUIRE(scm_object_eq(actual, expected), __VA_ARGS__)

#define REQUIRE_OBJ_EQ(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_OBJ_EQ, __VA_ARGS__)

#define REQUIRE_OBJ_EQV_ORIGIN(actual, expected, ...) \
    REQUIRE(scm_object_eqv(actual, expected), __VA_ARGS__)

#define REQUIRE_OBJ_EQV(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_OBJ_EQV, __VA_ARGS__)

#define REQUIRE_OBJ_EQUAL_ORIGIN(actual, expected, ...) \
    REQUIRE(scm_object_equal(actual, expected), __VA_ARGS__)

#define REQUIRE_OBJ_EQUAL(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_OBJ_EQUAL, __VA_ARGS__)

#define CHECK_EXC_ORIGIN(msg, exp, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        CHECK_SUBSTREQ(err, msg, strlen(msg), __VA_ARGS__); \
    } while (0)

#define CHECK_EXC(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_EXC, __VA_ARGS__)

#define CHECK_NOEXC_ORIGIN(exp, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        CHECK(!err, __VA_ARGS__); \
    } while (0);

#define CHECK_NOEXC(...) FIXED1_CHOOSER(__VA_ARGS__)(CHECK_NOEXC, __VA_ARGS__)

#define CHECK_NOEXC_EQ_ORIGIN(exp, val, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            CHECK_EQ(exp, val, __VA_ARGS__); \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        CHECK(!err, __VA_ARGS__); \
    } while (0)

#define CHECK_NOEXC_EQ(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_NOEXC_EQ, __VA_ARGS__)

#define CHECK_OBJ_EQ_ORIGIN(actual, expected, ...) \
    CHECK(scm_object_eq(actual, expected), __VA_ARGS__)

#define CHECK_OBJ_EQ(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_OBJ_EQ, __VA_ARGS__)

#define CHECK_OBJ_EQV_ORIGIN(actual, expected, ...) \
    CHECK(scm_object_eqv(actual, expected), __VA_ARGS__)

#define CHECK_OBJ_EQV(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_OBJ_EQV, __VA_ARGS__)

#define CHECK_OBJ_EQUAL_ORIGIN(actual, expected, ...) \
    CHECK(scm_object_equal(actual, expected), __VA_ARGS__)

#define CHECK_OBJ_EQUAL(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_OBJ_EQUAL, __VA_ARGS__)


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

