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

    /* valid templates */
    const char *t4[] = {
        "(b ...)", "((b) ...)", "((c ...) ...)", "(((c) ...) ...)", "((b c ...) ...)", "((c ... b) ... a)",
        "((a b) ...)", "((c ... a) ...)", "(((c a) ...) ...)", "((a c ...) ...)", "((c ... a) ...)",
        "#(b ...)", "#(#(b) ...)", "#(#(c ...) ...)", "#(#(#(c) ...) ...)", "#(#(b c ...) ...)", "#(#(c ... b) ... a)",
        "#(#(a b) ...)", "#(#(c ... a) ...)", "#(#(#(c a) ...) ...)", "#(#(a c ...) ...)", "#(#(c ... a) ...)",
    };
    n = sizeof(t4) / sizeof(const char *);
    exps = t4;
    for (int i = 0; i < n; ++i) {
        CHECK_NOEXC(check_template(read_exp(exps[i]), pids), "i=%d", i);
    }

    /* consecutive ellipses */
    const char *t5[] = {
        "(c ... ...)", "(a c ... ...)", "(c ... ... a)",
        "#(c ... ...)", "#(a c ... ...)", "#(c ... ... a)",
    };
    n = sizeof(t5) / sizeof(const char *);
    exps = t5;
    for (int i = 0; i < n; ++i) {
        CHECK_NOEXC(check_template(read_exp(exps[i]), pids), "i=%d", i);
    }

    /* less ellipses in template than in pattern */
    const char *t6[] = {
        "(((b c) ...) ...)", "#(#(#(b c) ...) ...)",
    };
    n = sizeof(t6) / sizeof(const char *);
    exps = t6;
    for (int i = 0; i < n; ++i) {
        CHECK_NOEXC(check_template(read_exp(exps[i]), pids), "i=%d", i);
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
    scm_object *pb;
    for (int i = 0; i < n; ++i) {
        CHECK(pids = match_pattern(read_exp(f3[i]), read_exp(p3[i]), scm_null, oenv, nenv), "i=%d", i);
        CHECK(pb = lookup_pid_binding(pids, SYM(a)), "i=%d", i);
        CHECK_EQ(pid_binding_get_depth(pb), 0, "i=%d", i);
        CHECK_OBJ_EQUAL(pid_binding_get_form(pb), read_exp(bf3[i]), "i=%d", i);
    }

    /* lists */
    const char *p4[] = {
        "(_)", "(_ ())", "(_ . 1)", "(_ 1 2)", "(_ #(1 2))",
        "(_)", "(_ ())", "(_ ())", "(_ ())", "(_ . 1)", "(_ . 1)", "(_ . 1)",
        "(_ 1 2)", "(_ 1 2)", "(_ 1 2)", "(_ a)", "(_ a)", "(_ 1)",
    };
    const char *f4[] = {
        "(_)", "(_ ())", "(_ . 1)", "(_ 1 2)", "(_ #(1 2))",
        "(_ 1)", "(_)", "(_ 1)", "(_ (1))",  "(_)",     "(_ . 2)", "(_ ())",
        "(_ 1)", "(_ 1 3)", "(_ 1 2 3)", "(_)", "(_ 1 2)", "(_ . 1)",
    };
    int res4[] = {
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        "(1 #())", "(1 #((0 1)))", "(1 #((0 1) (0 2)))", "(1 #())", "(1 #((0 x)))", "(1 #((0 x) (0 (2 y))))",
        "(2 #())", "(2 #((1 #())))",       "(2 #((1 #((0 x) (0 2)))))", "(2 #((1 #((0 o) (0 p) (0 x))) (1 #((0 2)))))",
    };
    n = sizeof(p5) / sizeof(const char *);

    for (int i = 0; i < n; ++i) {
        CHECK(pids = match_pattern(read_exp(f5[i]), read_exp(p5[i]), scm_null, oenv, nenv), "i=%d", i);
        CHECK(pb = lookup_pid_binding(pids, SYM(a)), "i=%d", i);
        CHECK_OBJ_EQUAL(pid_binding_get_binding(pb), read_exp(bf5[i]), "i=%d", i);
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
        "(1 #())", "(1 #((0 1)))", "(1 #((0 1) (0 2)))", "(1 #())", "(1 #((0 x)))", "(1 #((0 x) (0 #(2 y))))",
        "(2 #())", "(2 #((1 #())))",       "(2 #((1 #((0 x) (0 2)))))", "(2 #((1 #((0 o) (0 p) (0 x))) (1 #((0 2)))))",
    };
    n = sizeof(p8) / sizeof(const char *);

    for (int i = 0; i < n; ++i) {
        CHECK(pids = match_pattern(read_exp(f8[i]), read_exp(p8[i]), scm_null, oenv, nenv), "i=%d", i);
        CHECK(pb = lookup_pid_binding(pids, SYM(a)), "i=%d", i);
        CHECK_OBJ_EQUAL(pid_binding_get_binding(pb), read_exp(bf8[i]), "i=%d", i);
    }
}

TEST(xform, expand_template) {
    TEST_INIT();
    int n;
    scm_object *oenv = scm_global_env();
    scm_object *expanded;

    /* local identifiers in template */
    expanded = expand_template(read_exp("a"), scm_null, oenv);
    REQUIRE_EQ(expanded->type, scm_type_eidentifier);
    REQUIRE_EQ(scm_esymbol_get_uid(expanded), 1);
    REQUIRE_STREQ(scm_esymbol_get_string(expanded), "a");
    REQUIRE_EQ(scm_esymbol_get_env(expanded), oenv);

    expanded = expand_template(read_exp("(a a)"), scm_null, oenv);
    REQUIRE_EQ(expanded->type, scm_type_pair);
    REQUIRE_EQ(scm_esymbol_get_uid(scm_car(expanded)), 2);
    REQUIRE_STREQ(scm_esymbol_get_string(scm_car(expanded)), "a");
    REQUIRE_EQ(scm_esymbol_get_env(scm_car(expanded)), oenv);

    REQUIRE_EQ(scm_esymbol_get_uid(scm_cadr(expanded)), 2);
    REQUIRE_STREQ(scm_esymbol_get_string(scm_cadr(expanded)), "a");
    REQUIRE_EQ(scm_esymbol_get_env(scm_cadr(expanded)), oenv);

    /* list template */
    expanded = expand_template(read_exp("()"), scm_null, oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("()"));
    expanded = expand_template(read_exp("(1 #\\a \"abc\")"), scm_null, oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("(1 #\\a \"abc\")"));

    /* vector template */
    expanded = expand_template(read_exp("#()"), scm_null, oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("#()"));
    expanded = expand_template(read_exp("#(1 #\\a \"abc\")"), scm_null, oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("#(1 #\\a \"abc\")"));

    /* pattern identifiers in template */
    scm_object *pids = read_exp("((a 0 1) (b 0 (#t #f)))");
    expanded = expand_template(read_exp("a"), pids, oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("1"));
    expanded = expand_template(read_exp("b"), pids, oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("(#t #f)"));
    expanded = expand_template(read_exp("(0 a b)"), pids, oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("(0 1 (#t #f))"));
    expanded = expand_template(read_exp("#(0 a b)"), pids, oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("#(0 1 (#t #f))"));

    /* template beginning with ... */
    expanded = expand_template(read_exp("(... ...)"), scm_null, oenv);
    REQUIRE(same_id(expanded, read_exp("...")));

    expanded = expand_template(read_exp("(... (x ...))"), scm_null, oenv);
    REQUIRE_EQ(scm_list_length(expanded), 2);
    REQUIRE(same_id(scm_car(expanded), read_exp("x")));
    REQUIRE(same_id(scm_cadr(expanded), read_exp("...")));

    const char *bindings = "((a 1 #()) (b 1 #((0 1) (0 (1 2)))) (c 1 #((0 3) (0 4)))"
                            "(d 2 #()) (e 2 #((1 #()) (1 #((0 5) (0 6)))))"
                            "(x 0 #t))";
    const char *tests[] = {
        /*input form */             /* expanded */
        /* normal ellipses */
        "(a ...)",                  "()",
        "(9 b ...)",                "(9 1 (1 2))",
        "((9 b) ...)",              "((9 1) (9 (1 2)))",
        "((b c) ...)",              "((1 3) ((1 2) 4))",
        "((d ...) ...)",            "()",
        "((9 e ...) ...)",          "((9) (9 5 6))",
        "((b e ...) ...)",          "((1) ((1 2) 5 6))",
        "(((e) ...) ...)",          "(() ((5) (6)))",

        "#(a ...)",                 "#()",
        "#(9 b ...)",               "#(9 1 (1 2))",
        "#(#(9 b) ...)",            "#(#(9 1) #(9 (1 2)))",
        "#(#(b c) ...)",            "#(#(1 3) #((1 2) 4))",
        "#(#(d ...) ...)",          "#()",
        "#(#(9 e ...) ...)",        "#(#(9) #(9 5 6))",
        "#(#(b e ...) ...)",        "#(#(1) #((1 2) 5 6))",
        "#(#(#(e) ...) ...)",       "#(#() #(#(5) #(6)))",

        /* pid without following ellipses combined with ellipses followed pid */
        "(x a ...)",                "(#t)",
        "(x b ...)",                "(#t 1 (1 2))",
        "((x a) ...)",              "()",
        "((x b) ...)",              "((#t 1) (#t (1 2)))",
        "((x e ...) ...)",          "((#t) (#t 5 6))",
        "(((x e) ...) ...)",        "(() ((#t 5) (#t 6)))",

        "#(x a ...)",                "#(#t)",
        "#(x b ...)",                "#(#t 1 (1 2))",
        "#(#(x a) ...)",             "#()",
        "#(#(x b) ...)",             "#(#(#t 1) #(#t (1 2)))",
        "#(#(x e ...) ...)",         "#(#(#t) #(#t 5 6))",
        "#(#(#(x e) ...) ...)",      "#(#() #(#(#t 5) #(#t 6)))",
    };

    n = sizeof(tests) / sizeof(char *) / 2;
    for (int i = 0; i < n; ++i) {
        expanded = expand_template(read_exp(tests[2 * i]), read_exp(bindings), oenv);
        REQUIRE_OBJ_EQUAL(expanded, read_exp(tests[2 * i + 1]), "i=%d", i);
    }

    /* consecutive ellipses */
    bindings = "((a 2 #((1 #((0 1) (0 2))) (1 #((0 3) (0 4)))))"
                "(x 0 #t))";
    expanded = expand_template(read_exp("(a ... ...)"), read_exp(bindings), oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("(1 2 3 4)"));
    expanded = expand_template(read_exp("(x a ... ... x)"), read_exp(bindings), oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("(#t 1 2 3 4 #t)"));
    expanded = expand_template(read_exp("#(x a ... ... x)"), read_exp(bindings), oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("#(#t 1 2 3 4 #t)"));

    /* duplicate for the inner most ... */
    /* less ellipses in template than in pattern */
    bindings = "((a 1 #((0 1) (0 2))) (b 2 #((1 #((0 3) (0 4))) (1 #((0 5) (0 6)))))"
        "(c 3 #((2 #((1 #((0 7))) (1 #((0 8))))) (2 #((1 #((0 9))) (1 #((0 10))))))))";
    const char *tests2[] = {
        /*input form */                 /* expanded */
        "(((a b) ...) ...)",            "(((1 3) (1 4)) ((2 5) (2 6)))",
        "((((a c) ...) ...) ...)",      "((((1 7)) ((1 8))) (((2 9)) ((2 10))))",
        "(((a c ...) ...) ...)",        "(((1 7) (1 8)) ((2 9) (2 10)))",
        "((a c ... ...) ...)",          "((1 7 8) (2 9 10))",
        "((((b c) ...) ...) ...)",      "((((3 7)) ((4 8))) (((5 9)) ((6 10))))",
        "(((b c ...) ...) ...)",        "(((3 7) (4 8)) ((5 9) (6 10)))",

        "#(#(#(a b) ...) ...)",         "#(#(#(1 3) #(1 4)) #(#(2 5) #(2 6)))",
        "#(#(#(#(a c) ...) ...) ...)",  "#(#(#(#(1 7)) #(#(1 8))) #(#(#(2 9)) #(#(2 10))))",
        "#(#(#(a c ...) ...) ...)",     "#(#(#(1 7) #(1 8)) #(#(2 9) #(2 10)))",
        "#(#(a c ... ...) ...)",        "#(#(1 7 8) #(2 9 10))",
        "#(#(#(#(b c) ...) ...) ...)",  "#(#(#(#(3 7)) #(#(4 8))) #(#(#(5 9)) #(#(6 10))))",
        "#(#(#(b c ...) ...) ...)",     "#(#(#(3 7) #(4 8)) #(#(5 9) #(6 10)))",
    };

    n = sizeof(tests2) / sizeof(char *) / 2;
    for (int i = 0; i < n; ++i) {
        expanded = expand_template(read_exp(tests2[2 * i]), read_exp(bindings), oenv);
        REQUIRE_OBJ_EQUAL(expanded, read_exp(tests2[2 * i + 1]), "i=%d", i);
    }

    /* incompatible ellipsis match counts */
    bindings = "((a 1 #((0 1) (0 2))) (b 1 #((0 3))) (c 2 #((1 #((0 4) (0 5))))))";
    REQUIRE_EXC("expand_ellipses: incompatible ellipsis match counts for template",
                expand_template(read_exp("((a b) ...)"), read_exp(bindings), oenv));
    REQUIRE_EXC("expand_ellipses: incompatible ellipsis match counts for template",
                expand_template(read_exp("(((a c) ...) ...)"), read_exp(bindings), oenv));
    REQUIRE_EXC("expand_ellipses: incompatible ellipsis match counts for template",
                expand_template(read_exp("#(#(a b) ...)"), read_exp(bindings), oenv));
    REQUIRE_EXC("expand_ellipses: incompatible ellipsis match counts for template",
                expand_template(read_exp("#(#(#(a c) ...) ...)"), read_exp(bindings), oenv));

    /* duplicate for the outermost ... */
    ellipsis_duplicate_mode = 1;
    /* less ellipses in template than in pattern */
    bindings = "((a 1 #((0 1))) (b 2 #((1 #((0 2))) (1 #((0 3)))))"
        "(c 3 #((2 #((1 #((0 4))) (1 #((0 5))))) (2 #((1 #((0 6))) (1 #((0 7)))))))"
        "(d 2 #((1 #((0 11))) (1 #((0 12))))))";
    expanded = expand_template(read_exp("(((a b) ...) ...)"), read_exp(bindings), oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("(((1 2)) ((1 3)))"));
    expanded = expand_template(read_exp("((((a c) ...) ...) ...)"), read_exp(bindings), oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("((((1 4)) ((1 5))) (((1 6)) ((1 7))))"));
    expanded = expand_template(read_exp("((((b c) ...) ...) ...)"), read_exp(bindings), oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("((((2 4)) ((3 5))) (((2 6)) ((3 7))))"));
    expanded = expand_template(read_exp("#(#(#(a b) ...) ...)"), read_exp(bindings), oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("#(#(#(1 2)) #(#(1 3)))"));
    expanded = expand_template(read_exp("#(#(#(#(a c) ...) ...) ...)"), read_exp(bindings), oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("#(#(#(#(1 4)) #(#(1 5))) #(#(#(1 6)) #(#(1 7))))"));
    expanded = expand_template(read_exp("#(#(#(#(b c) ...) ...) ...)"), read_exp(bindings), oenv);
    REQUIRE_OBJ_EQUAL(expanded, read_exp("#(#(#(#(2 4)) #(#(3 5))) #(#(#(2 6)) #(#(3 7))))"));

    /* incompatible ellipsis match counts */
    bindings = "((a 1 #((0 1) (0 2))) (b 1 #((0 3))) (c 2 #((1 #((0 4))) (1 #((0 5))))))";
    REQUIRE_EXC("expand_ellipses: incompatible ellipsis match counts for template",
                expand_template(read_exp("((a b) ...)"), read_exp(bindings), oenv));
    REQUIRE_EXC("expand_ellipses: incompatible ellipsis match counts for template",
                expand_template(read_exp("(((a c) ...) ...)"), read_exp(bindings), oenv));
    REQUIRE_EXC("expand_ellipses: incompatible ellipsis match counts for template",
                expand_template(read_exp("#(#(a b) ...)"), read_exp(bindings), oenv));
    REQUIRE_EXC("expand_ellipses: incompatible ellipsis match counts for template",
                expand_template(read_exp("#(#(#(a c) ...) ...)"), read_exp(bindings), oenv));
}
