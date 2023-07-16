#include "test.h"

TAU_MAIN()

TEST(exp, self_evaluation) {
    int i, n;
    TEST_INIT();

    scm_object *trues[] = {
        scm_void, scm_true, scm_false, scm_chars['a'], scm_string_copy_new("abc", 3),
        scm_number_new_integer("1", 10), scm_number_new_float("1.0"), 
        scm_vector_new(1, scm_chars['1']), scm_vector_new(0),
    };

    scm_object *falses[] = {
        sym_if, scm_cons(scm_chars['1'], scm_chars['2']), scm_null,
    };
    /* eof, dot, rparen, port should not appear */

    n = sizeof(trues) / sizeof(scm_object *);

    for (i = 0; i < n; ++i) {
        CHECK(scm_exp_is_self_evaluation(trues[i]), "i=%d", i);
        scm_object_free(trues[i]);
    }

    n = sizeof(falses) / sizeof(scm_object *);
    for (i = 0; i < n; ++i) {
        CHECK(!scm_exp_is_self_evaluation(falses[i]), "i=%d", i);
        scm_object_free(falses[i]);
    }
}

TEST(exp, variable) {
    TEST_INIT();
    char buf[128];
    REQUIRE_NOEXC_EQ(scm_exp_is_variable(scm_symbol_new("abc", 3)), 1);

    for (int i = 0; i < keywords_num; ++i) {
        char *str = scm_symbol_get_string(keywords[i]);
        snprintf(buf, 128, "%s: bad syntax in: %s", str, str);
        REQUIRE_EXC(buf, scm_exp_is_variable(keywords[i]), "i=%d", i);
    }
    REQUIRE_NOEXC_EQ(scm_exp_is_variable(scm_true), 0);
}

TEST(exp, quote) {
    TEST_INIT();

    /* is quote */
    scm_object *q1 = scm_list(2, sym_quote, scm_true);
    REQUIRE_NOEXC_EQ(scm_exp_is_quote(q1), 1);
    REQUIRE_EQ(scm_exp_get_quote_text(q1), scm_true);
    scm_object_free(q1);

    /* not quote */
    scm_object *falses[] = {
        scm_true, scm_cons(scm_true, scm_false), scm_list(2, sym_if, scm_true),
    };
    int n = sizeof(falses) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC_EQ(scm_exp_is_quote(falses[i]), 0, "i=%d", i);
        scm_object_free(falses[i]);
    }
    
    /* invalid */
    scm_object *invalids[] = {
        scm_list(3, sym_quote, scm_true, scm_false),
        scm_cons(sym_quote, scm_true),
        scm_cons(sym_quote, scm_cons(scm_true, scm_false)),
    };
    char *msgs[] = {
        "quote: bad syntax in: (quote #t #f)",
        "quote: bad syntax in: (quote . #t)",
        "quote: bad syntax in: (quote #t . #f)",
    };
    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC(msgs[i], scm_exp_is_quote(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}

TEST(exp, assignment) {
    TEST_INIT();

    /* is assignment */
    scm_object *e1 = scm_list(3, sym_set, sym_do, scm_true);
    REQUIRE_NOEXC_EQ(scm_exp_is_assignment(e1), 1);
    REQUIRE_EQ(scm_exp_get_assignment_var(e1), sym_do);
    REQUIRE_EQ(scm_exp_get_assignment_val(e1), scm_true);
    scm_object_free(e1);

    /* not assignment */
    REQUIRE_NOEXC_EQ(scm_exp_is_assignment(scm_true), 0);
    
    /* invalid */
    scm_object *invalids[] = {
        scm_list(2, sym_set, scm_symbol_new("a", -1)),
        scm_cons(sym_set, scm_cons(scm_symbol_new("b", -1), scm_true)),
        scm_list(3, sym_set, scm_true, scm_true),
    };
    char *msgs[] = {
        "set!: bad syntax in: (set! a)",
        "set!: bad syntax in: (set! b . #t)",
        "set!: not an identifier in: #t",
    };
    int n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC(msgs[i], scm_exp_is_assignment(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}

TEST(exp, lambda) {
    TEST_INIT();

    /* not lambda */
    REQUIRE_NOEXC_EQ(scm_exp_is_lambda(scm_true), 0);

     /* is lambda */
    scm_object *params[] = {
        scm_symbol_new("x", -1),
        scm_null,
        scm_list(2, scm_symbol_new("a", -1), scm_symbol_new("b", -1)),
        scm_cons(scm_symbol_new("a", -1), scm_symbol_new("b", -1)),
    };
    scm_object *bodies[] = {
        scm_list(1, scm_true),
        scm_list(1, scm_true),
        scm_list(2, scm_true, scm_list(3, scm_symbol_new("cons", -1),
                                          scm_symbol_new("a", -1),
                                          scm_symbol_new("b", -1))),
        scm_list(1, scm_true),
    };
    scm_object *trues[] = {
        scm_cons(sym_lambda, scm_cons(params[0], bodies[0])),
        scm_cons(sym_lambda, scm_cons(params[1], bodies[1])),
        scm_cons(sym_lambda, scm_cons(params[2], bodies[2])),
        scm_cons(sym_lambda, scm_cons(params[3], bodies[3])),
    };

    int n = sizeof(trues) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC_EQ(scm_exp_is_lambda(trues[i]), 1, "i=%d", i);
        REQUIRE_EQ(scm_exp_get_lambda_parameters(trues[i]), params[i], "i=%d", i);
        REQUIRE_EQ(scm_exp_get_lambda_body(trues[i]), bodies[i], "i=%d", i);
        scm_object_free(trues[i]);
    }
   
    /* invalid */
    scm_object *invalids[] = {
        scm_list(1, sym_lambda),
        scm_list(2, sym_lambda, scm_true),
        scm_list(2, sym_lambda, scm_cons(scm_symbol_new("a", -1), scm_false)),
        scm_list(2, sym_lambda, scm_symbol_new("a", -1)),
        scm_list(2, sym_lambda, scm_list(1, scm_symbol_new("a", -1))),
    };
    char *msgs[] = {
        "lambda: bad syntax in: (lambda)",
        "lambda: not an identifier in: #t",
        "lambda: not an identifier in: #f",
        "lambda: no expression in body in: (lambda a)",
        "lambda: no expression in body in: (lambda (a))",
    };
    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC(msgs[i], scm_exp_is_lambda(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}

TEST(exp, definition) {
    TEST_INIT();

    /* false */
    REQUIRE_NOEXC_EQ(scm_exp_is_definition(scm_true), 0);

     /* true */
    scm_object *sym_x = scm_symbol_new("x", -1);
    scm_object *sym_foo = scm_symbol_new("foo", -1);
    scm_object *param1 = sym_x;
    scm_object *body1 = scm_list(1, scm_true);
    scm_object *param2 = scm_list(1, sym_x);
    scm_object *body2 = scm_list(2, scm_true, scm_false);
    scm_object *trues[] = {
        scm_list(3, sym_define, sym_x, scm_true),
        scm_cons(sym_define, scm_cons(scm_cons(sym_foo, param1), body1)),
        scm_cons(sym_define, scm_cons(scm_cons(sym_foo, param2), body2)),
    };
    scm_object *vars[] = {
        sym_x, sym_foo, sym_foo
    };
    scm_object *vals[] = {
        scm_true,
        scm_list(3, sym_lambda, sym_x, scm_true),
        scm_list(4, sym_lambda, scm_list(1, sym_x), scm_true, scm_false),
    };

    int n = sizeof(trues) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC_EQ(scm_exp_is_definition(trues[i]), 1, "i=%d", i);
        REQUIRE_OBJ_EQ(scm_exp_get_definition_var(trues[i]), vars[i], "i=%d", i);
        REQUIRE_OBJ_EQUAL(scm_exp_get_definition_val(trues[i]), vals[i], "i=%d", i);
        scm_object_free(trues[i]);
        scm_object_free(vals[i]);
    }
   
    /* invalid */
    scm_object *invalids[] = {
        scm_list(1, sym_define),
        scm_list(2, sym_define, scm_true),
        scm_list(2, sym_define, scm_null),
        scm_list(2, sym_define, scm_cons(scm_symbol_new("a", -1), scm_true)),
        scm_list(2, sym_define, scm_cons(scm_true, scm_symbol_new("a", -1))),
        scm_list(2, sym_define, scm_list(2, scm_symbol_new("a", -1), scm_true)),
        scm_list(2, sym_define, scm_list(2, scm_true, scm_symbol_new("a", -1))),
        scm_list(2, sym_define, scm_list(1, scm_symbol_new("a", -1))),
        scm_list(2, sym_define, scm_symbol_new("a", -1)),
        scm_list(4, sym_define, scm_symbol_new("a", -1), scm_true, scm_true),
    };
    char *msgs[] = {
        "define: bad syntax in: (define)",
        "define: bad syntax in: (define #t)",
        "define: bad syntax in: (define ())",
        "define: not an identifier in: #t",
        "define: not an identifier in: #t",
        "define: not an identifier in: #t",
        "define: not an identifier in: #t",
        "define: no expression in body in: (define (a))",
        "define: bad syntax (missing expression after identifier) in: (define a)",
        "define: bad syntax (multiple expression after identifier) in: (define a #t #t)",
    };
    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC(msgs[i], scm_exp_is_definition(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}

TEST(exp, if) {
    TEST_INIT();

    /* false */
    REQUIRE_NOEXC_EQ(scm_exp_is_if(scm_true), 0);

     /* true */
    scm_object *trues[] = {
        scm_list(4, sym_if, scm_true, scm_null, scm_false),
        scm_list(3, sym_if, scm_true, scm_null),
    };
    scm_object *tests[] = {
        scm_true, scm_true,
    };
    scm_object *conses[] = {
        scm_null, scm_null,
    };
    scm_object *alters[] = {
        scm_false, scm_void,
    };

    int n = sizeof(trues) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC_EQ(scm_exp_is_if(trues[i]), 1, "i=%d", i);
        REQUIRE_EQ(scm_exp_get_if_test(trues[i]), tests[i], "i=%d", i);
        REQUIRE_EQ(scm_exp_get_if_consequent(trues[i]), conses[i], "i=%d", i);
        REQUIRE_EQ(scm_exp_get_if_alternate(trues[i]), alters[i], "i=%d", i);
        scm_object_free(trues[i]);
    }
   
    /* invalid */
    scm_object *invalids[] = {
        scm_list(1, sym_if),
        scm_list(2, sym_if, scm_true),
        scm_list(5, sym_if, scm_true, scm_true, scm_true, scm_true),
    };
    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC("if: bad syntax in: ", scm_exp_is_if(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}

TEST(exp, begin) {
    TEST_INIT();

    /* false */
    REQUIRE_NOEXC_EQ(scm_exp_is_begin(scm_true), 0);

     /* true */
    scm_object *trues[] = {
        scm_list(2, sym_begin, scm_true),
        scm_list(3, sym_begin, scm_true, scm_false),
    };
    scm_object *exps[] = {
        scm_cdr(trues[0]), scm_cdr(trues[1]),
    };

    int n = sizeof(trues) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC_EQ(scm_exp_is_begin(trues[i]), 1, "i=%d", i);
        scm_object *seq = scm_exp_get_begin_sequence(trues[i]);
        REQUIRE_EQ(seq, exps[i], "i=%d", i);
        REQUIRE_EQ(scm_exp_get_sequence_first(seq), scm_car(exps[i]), "i=%d", i);
        REQUIRE_EQ(scm_exp_get_sequence_rest(seq), scm_cdr(exps[i]), "i=%d", i);
        REQUIRE_EQ(scm_exp_is_sequence_last(seq), scm_cdr(exps[i]) == scm_null, "i=%d", i);
        scm_object_free(trues[i]);
    }
   
    /* invalid */
    scm_object *invalids[] = {
        scm_list(1, sym_begin),
        scm_cons(sym_begin, scm_true),
    };
    char *errs[] = {
        "begin: no expressions in begin in",
        "begin: bad syntax in",
    };

    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC(errs[i], scm_exp_is_begin(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}

TEST(exp, qq) {
    TEST_INIT();

    /* false */
    REQUIRE_NOEXC_EQ(scm_exp_is_qq(scm_true), 0);

     /* true */
    scm_object *trues[] = {
        scm_list(2, sym_quasiquote, scm_true),
        scm_list(2, sym_quasiquote, scm_list(1, scm_true)),
    };
    scm_object *exps[] = {
        scm_cadr(trues[0]), scm_cadr(trues[1]),
    };

    int n = sizeof(trues) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC_EQ(scm_exp_is_qq(trues[i]), 1, "i=%d", i);
        REQUIRE_EQ(scm_exp_get_qq_exp(trues[i]), exps[i], "i=%d", i);
        scm_object_free(trues[i]);
    }
   
    /* invalid */
    scm_object *invalids[] = {
        scm_list(1, sym_quasiquote),
        scm_cons(sym_quasiquote, scm_true),
        scm_list(3, sym_quasiquote, scm_true, scm_true),
    };

    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC("quasiquote: bad syntax in", scm_exp_is_qq(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}

TEST(exp, unquote) {
    TEST_INIT();

    /* false */
    REQUIRE_NOEXC_EQ(scm_exp_is_unquote(scm_true), 0);

     /* true */
    scm_object *trues[] = {
        scm_list(2, sym_unquote, scm_true),
        scm_list(2, sym_unquote, scm_list(1, scm_true)),
    };
    scm_object *exps[] = {
        scm_cadr(trues[0]), scm_cadr(trues[1]),
    };

    int n = sizeof(trues) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC_EQ(scm_exp_is_unquote(trues[i]), 1, "i=%d", i);
        REQUIRE_EQ(scm_exp_get_unquote_exp(trues[i]), exps[i], "i=%d", i);
        scm_object_free(trues[i]);
    }
   
    /* invalid */
    scm_object *invalids[] = {
        scm_list(1, sym_unquote),
        scm_cons(sym_unquote, scm_true),
        scm_list(3, sym_unquote, scm_true, scm_true),
    };

    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC("unquote: bad syntax in", scm_exp_is_unquote(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}

TEST(exp, unquote_splicing) {
    TEST_INIT();

    /* false */
    REQUIRE_NOEXC_EQ(scm_exp_is_unquote_splicing(scm_true), 0);

     /* true */
    scm_object *trues[] = {
        scm_list(2, sym_unquote_splicing, scm_true),
        scm_list(2, sym_unquote_splicing, scm_list(1, scm_true)),
    };
    scm_object *exps[] = {
        scm_cadr(trues[0]), scm_cadr(trues[1]),
    };

    int n = sizeof(trues) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC_EQ(scm_exp_is_unquote_splicing(trues[i]), 1, "i=%d", i);
        REQUIRE_EQ(scm_exp_get_unquote_splicing_exp(trues[i]), exps[i], "i=%d", i);
        scm_object_free(trues[i]);
    }
   
    /* invalid */
    scm_object *invalids[] = {
        scm_list(1, sym_unquote_splicing),
        scm_cons(sym_unquote_splicing, scm_true),
        scm_list(3, sym_unquote_splicing, scm_true, scm_true),
    };

    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC("unquote-splicing: bad syntax in", scm_exp_is_unquote_splicing(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}

TEST(exp, application) {
    TEST_INIT();

    /* false */
    REQUIRE_NOEXC_EQ(scm_exp_is_application(scm_true), 0);

     /* true */
    scm_object *trues[] = {
        scm_list(1, scm_true),
        scm_list(2, scm_true, scm_true),
    };
    scm_object *opts[] = {
        scm_car(trues[0]), scm_car(trues[1]),
    };

    scm_object *opds[] = {
        scm_cdr(trues[0]), scm_cdr(trues[1]),
    };

    int n = sizeof(trues) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC_EQ(scm_exp_is_application(trues[i]), 1, "i=%d", i);
        REQUIRE_EQ(scm_exp_get_application_operator(trues[i]), opts[i], "i=%d", i);
        REQUIRE_EQ(scm_exp_get_application_operands(trues[i]), opds[i], "i=%d", i);
        scm_object_free(trues[i]);
    }
   
    /* invalid */
    scm_object *invalids[] = {
        scm_null,
        scm_cons(scm_true, scm_true),
    };

    char *errs[] = {
        "#%app: missing procedure expression in",
        "#%app: bad syntax in",
    };

    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC(errs[i], scm_exp_is_application(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}


