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

TEST(xform, match_pattern) {
    TEST_INIT();
    int n;
    scm_object *oenv = scm_global_env();
    scm_object *nenv = scm_env_extend(oenv, scm_list(1, sym_if), scm_list(1, scm_cons(scm_true, scm_false)));

    /* self evaluation */
    const char *p1[] = {
        "(_ 1)", "(_ 1)", "(_ \"ab\")", "(_ \"ab\")", "(_ #\\a)", "(_ #\\a)", "(_ #t)",  "(_ #t)",
    };
    const char *f1[] = {
        "(_ 1)", "(_ 2)", "(_ \"ab\")", "(_ \"abc\")", "(_ #\\a)", "(_ #\\b)", "(_ #t)", "(_ #f)",
    };
    int res1[] = {
        1,          0,        1,          0,            1,          0,          1,        0,
    };
    n = sizeof(p1) / sizeof(const char *);

    for (int i = 0; i < n; ++i) {
        CHECK_EQ(NULL != match_pattern(read_exp(f1[i]), read_exp(p1[i]), scm_null, oenv, nenv), res1[i], "i=%d", i);
    }


    /* literals */
    const char *p2[] = {
        "(_ set!)", "(_ x)", "(_ set!)", "(_ x)", "(_ if)",
    };
    const char *f2[] = {
        "(_ set!)", "(_ x)", "(_ x)",    "(_ y)", "(_ if)",
    };
    int res2[] = {
        1,          1,        0,          0,      0,
    };
    n = sizeof(p2) / sizeof(const char *);

    for (int i = 0; i < n; ++i) {
        CHECK_EQ(NULL != match_pattern(read_exp(f2[i]), read_exp(p2[i]), scm_list(3, sym_if, sym_set, SYM(x)), oenv, nenv), res2[i], "i=%d", i);
    }

    /* pattern variable matches any input form */
    const char *p3[] = {
        "(_ a)", "(_ a)", "(_ a)", "(_ a)",  "(_ a)",   "(_ a)",     "(_ a)",   "(_ a)",            "(_ . a)", "(_ . a)", "(_ . a)",
    };
    const char *f3[] = {
        "(_ 1)", "(_ a)", "(_ x)", "(_ ())", "(_ (1))", "(_ (a x))", "(_ #())", "(_ #(1 x (a x)))", "(_)",     "(_ 1)",   "(_ 1 (2 x))",
    };
    const char *bf3[] = {
        "1",     "a",     "x",     "()",     "(1)",     "(a x)",     "#()",     "#(1 x (a x))",     "()",      "(1)",     "(1 (2 x))",
    };
    n = sizeof(p3) / sizeof(const char *);

    scm_object *pids;
    scm_object *b;
    for (int i = 0; i < n; ++i) {
        CHECK(pids = match_pattern(read_exp(f3[i]), read_exp(p3[i]), scm_null, oenv, nenv), "i=%d", i);
        CHECK(b = lookup_pid_binding(pids, SYM(a)), "i=%d", i);
        CHECK_OBJ_EQV(binding_get_occurrence(b), scm_false, "i=%d", i);
        CHECK_OBJ_EQUAL(binding_get_form(b), read_exp(bf3[i]), "i=%d", i);
    }

    /* lists */
    const char *p4[] = {
        "(_)", "(_ ())", "(_ . 1)", "(_ 1 2)", "(_ #(1 2))",
        "(_)", "(_ ())", "(_ ())", "(_ ())", "(_ . 1)", "(_ . 1)", "(_ . 1)", "(_ 1 2)", "(_ 1 2)", "(_ 1 2)", "(_ a)", "(_ a)",
    };
    const char *f4[] = {
        "(_)", "(_ ())", "(_ . 1)", "(_ 1 2)", "(_ #(1 2))",
        "(_ 1)", "(_)", "(_ 1)", "(_ (1))",  "(_)",     "(_ . 2)", "(_ ())", "(_ 1)", "(_ 1 3)", "(_ 1 2 3)", "(_)", "(_ 1 2)",
    };
    int res4[] = {
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    n = sizeof(p4) / sizeof(const char *);

    for (int i = 0; i < n; ++i) {
        CHECK_EQ(NULL != match_pattern(read_exp(f4[i]), read_exp(p4[i]), scm_null, oenv, nenv), res4[i], "i=%d", i);
    }

    /* list ellipses */
    const char *p5[] = {
        "(_ a ...)", "(_ a ...)", "(_ a ...)", "(_ (a 1) ...)", "(_ (a 1) ...)", "(_ (a 1) ...)",
        "(_ (1 a ...) ...)", "(_ (1 a ...) ...)", "(_ (1 a ...) ...)", "(_ (1 a ...) ...)",
    };
    const char *f5[] = {
        "(_)",       "(_ 1)",     "(_ 1 2)",   "(_)",           "(_ (x 1))",     "(_ (x 1) ((2 y) 1))",
        "(_)",               "(_ (1))",           "(_ (1 x 2))",       "(_ (1 o p x) (1 2))",
    };
    const char *bf5[] = {
        "(0 ())", "(1 ((#f 1)))", "(2 ((#f 2) (#f 1)))", "(0 ())", "(1 ((#f x)))", "(2 ((#f (2 y)) (#f x)))",
        "(0 ())", "(1 ((0 ())))",       "(1 ((2 ((#f 2) (#f x)))))", "(2 ((1 ((#f 2))) (3 ((#f x) (#f p) (#f o)))))",
    };
    n = sizeof(p5) / sizeof(const char *);

    for (int i = 0; i < n; ++i) {
        CHECK(pids = match_pattern(read_exp(f5[i]), read_exp(p5[i]), scm_null, oenv, nenv), "i=%d", i);
        CHECK(b = lookup_pid_binding(pids, SYM(a)), "i=%d", i);
        CHECK_OBJ_EQUAL(b, read_exp(bf5[i]), "i=%d", i);
    }

    /* list ellipses mismatch */
    const char *p6[] = {
        "(_ a ...)", "(_ a ...)", "(_ (a 1) ...)", "(_ (a 1) ...)", "(_ (a 1) ...)", "(_ (a 1) ...)",
        "(_ (1 a ...) ...)", "(_ (1 a ...) ...)", "(_ (1 a ...) ...)", "(_ (1 a ...) ...)",
    };
    const char *f6[] = {
        "(_ . 1)",  "(_ 1 . 2)", "(_ (x 2))",   "(_ (x 1) (y))",    "(_ (x 1) . 2)", "(_ 1 1)",
        "(_ (2))",           "(_ (1 x) ())",      "(_ (1 x 2) (1 2 . 3))", "(_ (1 x y) . 1)",
    };
    n = sizeof(p6) / sizeof(const char *);

    for (int i = 0; i < n; ++i) {
        CHECK(NULL == match_pattern(read_exp(f6[i]), read_exp(p6[i]), scm_null, oenv, nenv), "i=%d", i);
    }

    /* vectors */
    const char *p7[] = {
        "(_ #())", "(_ #(1))", "(_ #(1 2))", "(_ #(#(1 2) 3))", "(_ #(1 (2 3)))",
        "(_ #())", "(_ #())", "(_ #())", "(_ #(1))", "(_ #(1))", "(_ #(1))", "(_ #(#(1 2) 3))", "(_ #(#(1 2) 3))",
    };
    const char *f7[] = {
        "(_ #())", "(_ #(1))", "(_ #(1 2))", "(_ #(#(1 2) 3))", "(_ #(1 (2 3)))",
        "(_)", "(_ 1)", "(_ #(1))",      "(_ #())", "(_ #(2))", "(_ #(1 2))", "(_ #(#(1 3) 3))", "(_ #(#(1 2) 3 4))",
    };
    int res7[] = {
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    n = sizeof(p7) / sizeof(const char *);

    for (int i = 0; i < n; ++i) {
        CHECK_EQ(NULL != match_pattern(read_exp(f7[i]), read_exp(p7[i]), scm_null, oenv, nenv), res7[i], "i=%d", i);
    }

    /* vector ellipses */
    const char *p8[] = {
        "(_ #(a ...))", "(_ #(a ...))", "(_ #(a ...))", "(_ #(a 1) ...)", "(_ #(a 1) ...)", "(_ #(a 1) ...)",
        "(_ #(1 a ...) ...)", "(_ #(1 a ...) ...)", "(_ #(1 a ...) ...)", "(_ #(1 a ...) ...)",
    };
    const char *f8[] = {
        "(_ #())",       "(_ #(1))",     "(_ #(1 2))",   "(_)",          "(_ #(x 1))",     "(_ #(x 1) #(#(2 y) 1))",
        "(_)",               "(_ #(1))",           "(_ #(1 x 2))",       "(_ #(1 o p x) #(1 2))",
    };
    const char *bf8[] = {
        "(0 ())", "(1 ((#f 1)))", "(2 ((#f 2) (#f 1)))", "(0 ())", "(1 ((#f x)))", "(2 ((#f #(2 y)) (#f x)))",
        "(0 ())", "(1 ((0 ())))",       "(1 ((2 ((#f 2) (#f x)))))", "(2 ((1 ((#f 2))) (3 ((#f x) (#f p) (#f o)))))",
    };
    n = sizeof(p8) / sizeof(const char *);

    for (int i = 0; i < n; ++i) {
        CHECK(pids = match_pattern(read_exp(f8[i]), read_exp(p8[i]), scm_null, oenv, nenv), "i=%d", i);
        CHECK(b = lookup_pid_binding(pids, SYM(a)), "i=%d", i);
        CHECK_OBJ_EQUAL(b, read_exp(bf8[i]), "i=%d", i);
    }

}
