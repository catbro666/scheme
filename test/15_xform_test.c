#include "test.h"

TAU_MAIN()

scm_object *read_exp(const char *exp) {
    scm_object *port = string_input_port_new(exp, -1);
    scm_object *o = scm_read(port);
    scm_object_free(port);
    return o;
}

TEST(xform, check_pattern) {
    TEST_INIT();
    int n;
    const char **exps;

    /* top level pattern isn't a list-structed form beginging with keyword */
    const char *t1[] = {
        "()", "a", "#()", "#(_ 2)",
    };
    n = sizeof(t1) / sizeof(const char *);
    exps = t1;


    for (int i = 0; i < n; ++i) {
        CHECK_EXC("syntax-rules: the outermost <pattern> must be a list-structured form",
                  check_pattern(read_exp(exps[i]), scm_null), "i=%d", i);
    }

    /* `...` isn't following a subpattern including pattern identifiers */
    const char *t2[] = {
        "(_ ...)", "(_ 1 ...)", "(_ l ...)", "(_ () ...)", "(_ (#\\a) ...)",
        "(_ (l) ...)", "(_ #() ...)", "(_ #(1) ...)", "(_ #(l) ...)",
        "(_ #(...))", "(_ #(1 ...))", "(_ #(l ...))", "(_ #(() ...))", "(_ #((#\\a) ...))",
        "(_ #((l) ...))", "(_ #(#() ...))", "(_ #(#(1) ...))", "(_ #(#(l) ...))",
    };
    n = sizeof(t2) / sizeof(const char *);
    exps = t2;

    for (int i = 0; i < n; ++i) {
        CHECK_EXC("syntax-rules: the `...` must be after subpattern including pattern identifiers",
                  check_pattern(read_exp(exps[i]), scm_list(1, SYM(l))), "i=%d", i);
    }

    /* `...` isn't at the end of a <pattern> in the sequence form */
    const char *t3[] = {
        "(_ a ... b)", "(_ a ... 1)", "(_ a ... ...)", "(_ a . ...)",
        "(_ #(a ... b))", "(_ #(a ... 1))", "(_ #(a ... ...))",
    };
    n = sizeof(t3) / sizeof(const char *);
    exps = t3;

    for (int i = 0; i < n; ++i) {
        CHECK_EXC("syntax-rules: the `...` must be at the end of a <pattern> in the sequence form",
                  check_pattern(read_exp(exps[i]), scm_null), "i=%d", i);
    }

    /* variable used twice in pattern */
    const char *t4[] = {
        "(_ a a)", "(_ a (a))", "(_ a #(a))", "(_ #(a a))", "(_ #(a #(a)))", "(_ #(a (a)))",
    };
    n = sizeof(t4) / sizeof(const char *);
    exps = t4;

    for (int i = 0; i < n; ++i) {
        CHECK_EXC("syntax-rules: variable used twice in pattern",
                  check_pattern(read_exp(exps[i]), scm_null), "i=%d", i);
    }

    /* valid patterns followed by ellipsis */
    const char *t5[] = {
        "(_ (a ...) (b ...) ...)", "(_ #(a ...) #(b ...) ...)",
    };
    n = sizeof(t5) / sizeof(const char *);
    exps = t5;

    scm_object *pids;
    scm_object *o;
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC(pids = check_pattern(read_exp(exps[i]), scm_null), "i=%d", i);
        o = scm_assq(SYM(a), pids);
        REQUIRE_EQ(o->type, scm_type_pair);
        REQUIRE_OBJ_EQV(scm_cdr(o), INTEGER(1));
        o = scm_assq(SYM(b), pids);
        REQUIRE_EQ(o->type, scm_type_pair);
        REQUIRE_OBJ_EQV(scm_cdr(o), INTEGER(2));
    }
}

TEST(xform, check_template) {
    TEST_INIT();
    int n;
    const char **exps;

    /* single `...` or list template beginning with `...`: (... <template>) */
    const char *t1[] = {
        "...", "(...)", "(... . 1)", "(... ... 1)", "(... 1 . 1)",
    };
    n = sizeof(t1) / sizeof(const char *);
    exps = t1;

    for (int i = 0; i < n; ++i) {
        CHECK_EXC("syntax-rules: illegal use of `...` in template",
                  check_template(read_exp(exps[i]), scm_null), "i=%d", i);
    }

    /* too few ellipses */
    scm_object *pids = scm_list(3, scm_cons(SYM(a), INTEGER(0)),
                                   scm_cons(SYM(b), INTEGER(1)),
                                   scm_cons(SYM(c), INTEGER(2)));
    const char *t2[] = {
        "(b)", "(c)", "(c ...)", "(c) ...", "(b c ... ...)", "(b ... c ...)",
        "#(b)", "#(c)", "#(c ...)", "#(c) ...", "#(b c ... ...)", "#(b ... c ...)",
    };
    n = sizeof(t2) / sizeof(const char *);
    exps = t2;

    for (int i = 0; i < n; ++i) {
        CHECK_EXC("syntax-rules: pattern variables that occur in subpattern followed by one or more",
                  check_template(read_exp(exps[i]), pids), "i=%d", i);
    }

    /* too many ellipses */
    const char *t3[] = {
        "(b ... ...)", "((b ...) ...)", "((b) ... ...)", "(((b) ...) ...)", "((1 . b) ... ...)",
        "(c ... ... ...)", "((c ...) ... ...)", "(1 ...)", "(l ...)", "(a ...)", "((1 . a) ...)",
        "#(b ... ...)", "#((b ...) ...)", "#((b) ... ...)", "#(((b) ...) ...)",
        "#(c ... ... ...)", "#((c ...) ... ...)",
        "#(1 ...)", "#(l ...)", "#(a ...)", "((1 . #(a)) ...)",
    };
    n = sizeof(t3) / sizeof(const char *);
    exps = t3;

    for (int i = 0; i < n; ++i) {
        CHECK_EXC("syntax-rules: subtemplates that are followed by `...` must contain at least 1 pattern variable",
                  check_template(read_exp(exps[i]), pids), "i=%d", i);
    }
}
