#include "test.h"

TAU_MAIN()

TEST(exp, self_evaluation) {
    int i, n;
    TEST_INIT();

    scm_object *trues[] = {
        scm_void, scm_true, scm_false, scm_chars['a'], scm_string_copy_new("abc", 3),
        scm_number_new_integer("1", 10), scm_number_new_float("1.0"), 
    };

    scm_object *falses[] = {
        scm_empty_vector, scm_vector_new(1, scm_chars['1']), sym_if,
        scm_cons(scm_chars['1'], scm_chars['2']), scm_null,
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

TEST(exp, quote) {
    TEST_INIT();

    /* valid */
    scm_object *q1 = scm_list(2, sym_quote, scm_true);
    REQUIRE_NOEXC(scm_exp_check_quote(q1));
    REQUIRE_EQ(scm_exp_get_quote_text(q1), scm_true);
    scm_object_free(q1);

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
    int n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC(msgs[i], scm_exp_check_quote(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, assignment) {
    TEST_INIT();

    /* valid */
    scm_object *e1 = scm_list(3, sym_set, sym_do, scm_true);
    REQUIRE_NOEXC(scm_exp_check_assignment(e1));
    REQUIRE_EQ(scm_exp_get_assignment_var(e1), sym_do);
    REQUIRE_EQ(scm_exp_get_assignment_val(e1), scm_true);
    scm_object_free(e1);

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
        REQUIRE_EXC(msgs[i], scm_exp_check_assignment(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, lambda) {
    TEST_INIT();

     /* valid */
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
        REQUIRE_NOEXC(scm_exp_check_lambda(trues[i]), "i=%d", i);
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
        REQUIRE_EXC(msgs[i], scm_exp_check_lambda(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, definition) {
    TEST_INIT();

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
        REQUIRE_NOEXC(scm_exp_check_definition(trues[i]), "i=%d", i);
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
        REQUIRE_EXC(msgs[i], scm_exp_check_definition(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, if) {
    TEST_INIT();

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
        REQUIRE_NOEXC(scm_exp_check_if(trues[i]), "i=%d", i);
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
        REQUIRE_EXC("if: bad syntax in: ", scm_exp_check_if(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, begin) {
    TEST_INIT();

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
        REQUIRE_NOEXC(scm_exp_check_begin(trues[i]), "i=%d", i);
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
        REQUIRE_EXC(errs[i], scm_exp_check_begin(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, qq) {
    TEST_INIT();

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
        REQUIRE_NOEXC(scm_exp_check_qq(trues[i]), "i=%d", i);
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
        REQUIRE_EXC("quasiquote: bad syntax in", scm_exp_check_qq(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, unquote) {
    TEST_INIT();

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
        REQUIRE_NOEXC(scm_exp_check_unquote(trues[i]), "i=%d", i);
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
        REQUIRE_EXC("unquote: bad syntax in", scm_exp_check_unquote(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, unquote_splicing) {
    TEST_INIT();

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
        REQUIRE_NOEXC(scm_exp_check_unquote_splicing(trues[i]), "i=%d", i);
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
        REQUIRE_EXC("unquote-splicing: bad syntax in", scm_exp_check_unquote_splicing(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, application) {
    TEST_INIT();

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
        REQUIRE_NOEXC(scm_exp_check_application(trues[i]), "i=%d", i);
        REQUIRE_EQ(scm_exp_get_application_operator(trues[i]), opts[i], "i=%d", i);
        REQUIRE_EQ(scm_exp_get_application_operands(trues[i]), opds[i], "i=%d", i);
        scm_object_free(trues[i]);
    }
   
    /* invalid */
    scm_object *invalids[] = {
        scm_cons(scm_true, scm_true),
    };

    char *errs[] = {
        "#%app: bad syntax in",
    };

    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC(errs[i], scm_exp_check_application(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, let_syntax) {
    TEST_INIT();

     /* true */
    scm_object *exp1 = scm_list(3, sym_let_syntax, scm_null, scm_true);
    scm_object *exp2 = scm_list(3, sym_let_syntax, scm_list(1,
            scm_list(2, SYM(a), scm_list(3, SYM(syntax-rules), scm_null,
                                         scm_list(2, scm_list(1, sym_underscore), scm_true)))), scm_true);
    scm_object *trues[] = {
        exp1, exp2,
    };
    int n = sizeof(trues) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC(scm_exp_check_let_syntax(trues[i]), "i=%d", i);
        REQUIRE_EQ(scm_exp_get_syntax_bindings(trues[i]), scm_cadr(trues[i]), "i=%d", i);
        REQUIRE_EQ(scm_exp_get_let_syntax_body(trues[i]), scm_cddr(trues[i]), "i=%d", i);
        scm_object_free(trues[i]);
    }

    /* invalid */
    scm_object *invalids[] = {
        scm_list(2, sym_let_syntax, scm_null),
    };

    n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC("let-syntax: bad syntax in", scm_exp_check_let_syntax(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, macro_binding) {
    TEST_INIT();

    /* invalid */
    scm_object *invalids[] = {
        scm_list(2, sym_define_syntax, SYM(a)),
        scm_list(4, sym_define_syntax, SYM(a), scm_true, scm_true),
        scm_list(3, sym_define_syntax, scm_true, scm_true),
    };

    char *errs[] = {
        "macro-binding: must be the form of `(<keyword> <transformer spec>)` in",
        "macro-binding: must be the form of `(<keyword> <transformer spec>)` in",
        "macro-binding: <keyword> must be an identifier in:",
    };


    int n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC(errs[i], scm_exp_check_define_syntax(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    }
}

TEST(exp, syntax_rules) {
    TEST_INIT();

    /* invalid */
    scm_object *invalids[] = {
        scm_list(3, sym_define_syntax, SYM(a), scm_list(1, SYM(syntax-rules))),
        scm_list(3, sym_define_syntax, SYM(a), scm_list(2, SYM(a), scm_true)),
        scm_list(3, sym_define_syntax, SYM(a), scm_list(2, SYM(syntax-rules), scm_true)),
        scm_list(3, sym_define_syntax, SYM(a), scm_list(2, SYM(syntax-rules), scm_list(1, scm_true))),
        scm_list(3, sym_define_syntax, SYM(a), scm_list(2, SYM(syntax-rules), scm_list(1, sym_ellipsis))),
        scm_list(3, sym_define_syntax, SYM(a), scm_list(2, SYM(syntax-rules), scm_list(1, sym_underscore))),
    };

    char *errs[] = {
        "syntax-rules: bad syntax in",
        "syntax-rules: only a `syntax-rules' form is allowed in",
        "syntax-rules: <literals> must be a list of identifiers",
        "syntax-rules: <literals> must be a list of identifiers",
        "syntax-rules: `...` is not allowed in <literals>",
        "syntax-rules: `_` is not allowed in <literals>",
    };


    int n = sizeof(invalids) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_EXC(errs[i], scm_exp_check_define_syntax(invalids[i]), "i=%d", i);
        scm_object_free(invalids[i]);
    } 
}
