#include "../src/err.h"
#include <tau/tau.h>
#include <string.h>

TAU_MAIN()

TEST(err, no_exception) {
    int i = 0;

    REQUIRE(!scm_jmp, "initially NULL");

    SCM_TRY {
        REQUIRE(scm_jmp, "setjmp in TRY block");
        i++;
    } SCM_CATCH {
        REQUIRE(0, "should not reach here");
    } SCM_END_TRY;

    REQUIRE_EQ(i, 1);
    REQUIRE(!scm_jmp, "restore scm_jmp to the previous value");
}

TEST(err, throw_exception) {
    char *msg = NULL;
    char *err_msg = "ERROR!!!";

    REQUIRE(!scm_jmp, "initially NULL");

    SCM_TRY {
        REQUIRE(scm_jmp, "setjmp in TRY block");
        scm_error("%s", err_msg);
        REQUIRE(0, "should not reach here");
    } SCM_CATCH {
        REQUIRE(!scm_jmp, "restore scm_jmp to the previous value");
        msg = scm_error_msg();
    } SCM_END_TRY;

    REQUIRE_STREQ(msg, err_msg);
    REQUIRE(!scm_jmp, "restore scm_jmp to the previous value");
}

TEST(err, nested_try_catch) {
    char *in_msg = NULL;
    char *out_msg = NULL;
    char *in_err = "Inner Exception";
    char *out_err = "Outer Exception";
    char buf[1024] = {0};

    REQUIRE(!scm_jmp, "initially NULL");

    SCM_TRY {
        jmp_buf *j1 = scm_jmp;
        REQUIRE(scm_jmp, "setjmp in TRY block");
        SCM_TRY {
            REQUIRE(scm_jmp, "setjmp in TRY block");
            REQUIRE(scm_jmp != j1, "setjmp in TRY block");
            scm_error("%s", in_err);
            SCM_THROW;
            REQUIRE(0, "should not reach here");
        } SCM_CATCH {
            REQUIRE(scm_jmp == j1, "restore scm_jmp to the previous value");
            in_msg = scm_error_msg();
            strncpy(buf, in_msg, sizeof(buf));
            scm_error("%s", out_err);
            SCM_THROW;
            REQUIRE(0, "should not reach here");
        } SCM_END_TRY;
    } SCM_CATCH {
        REQUIRE(!scm_jmp, "restore scm_jmp to the previous value");
        out_msg = scm_error_msg();
    } SCM_END_TRY;

    REQUIRE_STREQ(buf, in_err);
    REQUIRE_STREQ(out_msg, out_err);
    REQUIRE(!scm_jmp, "restore scm_jmp to the initial value");
}

void my_free(void *p) {
    (*((int *)p))++;
}

TEST(err, scm_error_free) {
    char *msg = NULL;
    char *err_msg = "ERROR!!!";
    int x = 1;

    REQUIRE(!scm_jmp, "initially NULL");

    SCM_TRY {
        REQUIRE(scm_jmp, "setjmp in TRY block");
        scm_error_free(my_free, &x, "%s", err_msg);
        REQUIRE(0, "should not reach here");
    } SCM_CATCH {
        REQUIRE(!scm_jmp, "restore scm_jmp to the previous value");
        msg = scm_error_msg();
    } SCM_END_TRY;

    REQUIRE_STREQ(msg, err_msg);
    REQUIRE_EQ(x, 2);   /* free_fn is called */
    REQUIRE(!scm_jmp, "restore scm_jmp to the previous value");
}
