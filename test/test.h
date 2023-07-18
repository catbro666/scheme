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
#include "../src/exp.h"
#include "../src/read.h"
#include "../src/write.h"
#include "../src/env.h"
#include "../src/eval.h"
#include "../src/proc.h"

#include <tau/tau.h>
#include <limits.h>

/* @msg: the expected error msg
 * @exp: the expression that is expected to throw exception */
#define REQUIRE_EXC_(msg, exp, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        REQUIRE_SUBSTREQ_(err, msg, strlen(msg), __VA_ARGS__); \
    } while (0)

#define REQUIRE_EXC(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_EXC, __VA_ARGS__)

#define REQUIRE_NOEXC_(exp, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        REQUIRE_(!err, __VA_ARGS__); \
    } while (0)

#define REQUIRE_NOEXC(...) FIXED1_CHOOSER(__VA_ARGS__)(REQUIRE_NOEXC, __VA_ARGS__)

#define REQUIRE_NOEXC_EQ_(exp, val, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            REQUIRE_EQ_(exp, val, __VA_ARGS__); \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        REQUIRE_(!err, __VA_ARGS__); \
    } while (0)

#define REQUIRE_NOEXC_EQ(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_NOEXC_EQ, __VA_ARGS__)

#define REQUIRE_OBJ_EQ_(actual, expected, ...) \
    REQUIRE_(scm_eq(actual, expected), __VA_ARGS__)

#define REQUIRE_OBJ_EQ(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_OBJ_EQ, __VA_ARGS__)

#define REQUIRE_OBJ_EQV_(actual, expected, ...) \
    REQUIRE_(scm_eqv(actual, expected), __VA_ARGS__)

#define REQUIRE_OBJ_EQV(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_OBJ_EQV, __VA_ARGS__)

#define REQUIRE_OBJ_EQUAL_(actual, expected, ...) \
    REQUIRE_(scm_equal(actual, expected), __VA_ARGS__)

#define REQUIRE_OBJ_EQUAL(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_OBJ_EQUAL, __VA_ARGS__)

#define REQUIRE_OBJ_NEQ_(actual, expected, ...) \
    REQUIRE_(!scm_eq(actual, expected), __VA_ARGS__)

#define REQUIRE_OBJ_NEQ(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_OBJ_EQ, __VA_ARGS__)

#define REQUIRE_OBJ_NEQV_(actual, expected, ...) \
    REQUIRE_(!scm_eqv(actual, expected), __VA_ARGS__)

#define REQUIRE_OBJ_NEQV(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_OBJ_EQV, __VA_ARGS__)

#define REQUIRE_OBJ_NEQUAL_(actual, expected, ...) \
    REQUIRE_(!scm_equal(actual, expected), __VA_ARGS__)

#define REQUIRE_OBJ_NEQUAL(...) FIXED2_CHOOSER(__VA_ARGS__)(REQUIRE_OBJ_EQUAL, __VA_ARGS__)

#define CHECK_EXC_(msg, exp, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        CHECK_SUBSTREQ_(err, msg, strlen(msg), __VA_ARGS__); \
    } while (0)

#define CHECK_EXC(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_EXC, __VA_ARGS__)

#define CHECK_NOEXC_(exp, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            exp; \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        CHECK_(!err, __VA_ARGS__); \
    } while (0);

#define CHECK_NOEXC(...) FIXED1_CHOOSER(__VA_ARGS__)(CHECK_NOEXC, __VA_ARGS__)

#define CHECK_NOEXC_EQ_(exp, val, ...) \
    do { \
        char *err = NULL; \
        SCM_TRY { \
            CHECK_EQ_(exp, val, __VA_ARGS__); \
        } SCM_CATCH { \
            err = scm_error_msg(); \
        } SCM_END_TRY; \
 \
        CHECK_(!err, __VA_ARGS__); \
    } while (0)

#define CHECK_NOEXC_EQ(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_NOEXC_EQ, __VA_ARGS__)

#define CHECK_OBJ_EQ_(actual, expected, ...) \
    CHECK_(scm_eq(actual, expected), __VA_ARGS__)

#define CHECK_OBJ_EQ(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_OBJ_EQ, __VA_ARGS__)

#define CHECK_OBJ_EQV_(actual, expected, ...) \
    CHECK_(scm_eqv(actual, expected), __VA_ARGS__)

#define CHECK_OBJ_EQV(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_OBJ_EQV, __VA_ARGS__)

#define CHECK_OBJ_EQUAL_(actual, expected, ...) \
    CHECK_(scm_equal(actual, expected), __VA_ARGS__)

#define CHECK_OBJ_EQUAL(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_OBJ_EQUAL, __VA_ARGS__)

#define CHECK_OBJ_NEQ_(actual, expected, ...) \
    CHECK_(!scm_eq(actual, expected), __VA_ARGS__)

#define CHECK_OBJ_NEQ(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_OBJ_NEQ, __VA_ARGS__)

#define CHECK_OBJ_NEQV_(actual, expected, ...) \
    CHECK_(!scm_eqv(actual, expected), __VA_ARGS__)

#define CHECK_OBJ_NEQV(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_OBJ_NEQV, __VA_ARGS__)

#define CHECK_OBJ_NEQUAL_(actual, expected, ...) \
    CHECK_(!scm_equal(actual, expected), __VA_ARGS__)

#define CHECK_OBJ_NEQUAL(...) FIXED2_CHOOSER(__VA_ARGS__)(CHECK_OBJ_NEQUAL, __VA_ARGS__)


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
        res = scm_exp_init(); \
        REQUIRE(!res, "scm_exp_init"); \
        res = scm_proc_init(); \
        REQUIRE(!res, "scm_proc_init"); \
    } while (0)

#endif /* SCHEME_TEST_H */

