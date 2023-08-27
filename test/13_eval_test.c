#include "test.h"

TAU_MAIN()

// vector, null

TEST(eval, self_evaluation) {
    scm_object *o = NULL;
    TEST_INIT();

    scm_object *env = scm_env_new();

    scm_object *str = scm_string_copy_new("abc", 3);
    scm_object *ii = scm_number_new_integer("1", 10);
    scm_object *ff = scm_number_new_float("1.0");

    scm_object *objs[] = {
        scm_void, scm_true, scm_false, scm_chars['a'], str, ii, ff,
    };

    int n = sizeof(objs) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        o = scm_eval(objs[i], env);
        CHECK(o, "scm_eval(objs[i]), i=%d", i);
        CHECK_EQ(o, objs[i], "i=%d", i);
        scm_object_free(objs[i]);
    }

    scm_object_free(env);
}

TEST(eval, vector) {
    scm_object *o = NULL;
    TEST_INIT();

    scm_object *env = scm_env_new();

    scm_object *vec = scm_vector_new(2, scm_true, SYM(a));

    scm_object *objs[] = {
        scm_empty_vector, vec,
    };

    int n = sizeof(objs) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        o = scm_eval(objs[i], env);
        CHECK(o, "scm_eval(objs[i]), i=%d", i);
        CHECK_OBJ_EQUAL(o, objs[i], "i=%d", i);
        scm_object_free(objs[i]);
    }

    scm_object_free(env);
}

TEST(eval, variable) {
    scm_object *o = NULL;
    TEST_INIT();

    scm_object *env = scm_global_env();
    scm_object *a = SYM(a);
    scm_object *b = SYM(b);
    env = scm_env_extend(env, scm_list(1, a), scm_list(1, scm_true));

    /* defined variable */
    REQUIRE((o = scm_eval(a, env)), "scm_eval(a, env)");
    REQUIRE_EQ(o, scm_true);

    /* undefined variable */
    REQUIRE_EXC("b: undefined;\n", scm_eval(b, env));

    /* syntax keyword */
    REQUIRE_EXC("if: bad syntax in", scm_eval(sym_if, env));
}

TEST(eval, quote) {
    scm_object *o = NULL;
    TEST_INIT();

    scm_object *env = scm_global_env();
    scm_object *a = SYM(a);
    env = scm_env_extend(env, scm_list(1, a), scm_list(1, scm_cons(scm_true, scm_false)));

    scm_object *objs[] = {
        scm_void, scm_true, scm_cons(a, scm_false),
    };
    scm_object *exps[] = {
        scm_list(2, sym_quote, objs[0]),
        scm_list(2, sym_quote, objs[1]), 
        scm_list(2, sym_quote, objs[2]),
    };

    int n = sizeof(objs) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        CHECK_OBJ_EQUAL((o = scm_eval(exps[i], env)), objs[i], "i=%d", i);
        scm_object_free(objs[i]);
    }

}

TEST(eval, assignment) {
    TEST_INIT();

    scm_object *env = scm_global_env();
    scm_object *a = SYM(a);
    scm_object *b = SYM(b);
    env = scm_env_extend(env, scm_list(1, a), scm_list(1, scm_true));

    /* defined variable */
    scm_object *exp1 = scm_list(3, sym_set, a, scm_false);
    REQUIRE_EQ(scm_eval(exp1, env), scm_void);
    REQUIRE_EQ(scm_env_lookup_var(env, a), scm_false);

    /* undefined variable */
    scm_object *exp2 = scm_list(3, sym_set, b, scm_false);
    REQUIRE_EXC("set!: assignment disallowed;", scm_eval(exp2, env));

    scm_object_free(exp1);
    scm_object_free(exp2);
}

TEST(eval, lambda) {
    scm_object *o = NULL;
    TEST_INIT();

    scm_object *env = scm_global_env();

    scm_object *exps[] = {
        scm_list(3, sym_lambda, scm_null, scm_true),
        scm_list(3, sym_lambda, SYM(a), scm_true),
        scm_list(3, sym_lambda, scm_cons(SYM(a), SYM(b)), scm_true),
        scm_list(3, sym_lambda, scm_list(2, SYM(a), SYM(b)), scm_true),
    };

    int n = sizeof(exps) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        o = scm_eval(exps[i], env);
        CHECK_NOEXC_EQ(o->type, scm_type_compound, "i=%d", i);
        scm_object_free(exps[i]);
    }
}

TEST(eval, definition) {
    TEST_INIT();

    scm_object *env = scm_global_env();
    scm_object *a = SYM(a);
    scm_object *b = SYM(b);
    env = scm_env_extend(env, scm_list(1, a), scm_list(1, scm_true));

    /* defined variable */
    scm_object *exp1 = scm_list(3, sym_define, a, scm_false);
    REQUIRE_EQ(scm_eval(exp1, env), scm_void);
    REQUIRE_EQ(scm_env_lookup_var(env, a), scm_false);

    /* undefined variable */
    scm_object *exp2 = scm_list(3, sym_define, b, scm_false);
    REQUIRE_EQ(scm_eval(exp2, env), scm_void);
    REQUIRE_EQ(scm_env_lookup_var(env, b), scm_false);

    /* define procedure */

    scm_object_free(exp1);
    scm_object_free(exp2);
}

TEST(eval, if) {
    TEST_INIT();

    scm_object *env = scm_global_env();
    scm_object *a = SYM(a);
    env = scm_env_extend(env, scm_list(1, a), scm_list(1, scm_true));

    /* consequent branch */
    scm_object *exp1 = scm_list(3, sym_if, scm_true, scm_chars['a']);
    REQUIRE_EQ(scm_eval(exp1, env), scm_chars['a']);
    scm_object *exp2 = scm_list(4, sym_if, scm_true, scm_chars['a'], scm_list(3, sym_set, a, scm_false));
    REQUIRE_EQ(scm_eval(exp2, env), scm_chars['a']);
    /* the alternate didn't get evaluated */
    REQUIRE_EQ(scm_env_lookup_var(env, a), scm_true);

    /* alternate branch */
    scm_object *exp3 = scm_list(3, sym_if, scm_false, scm_list(3, sym_set, a, scm_false));
    REQUIRE_EQ(scm_eval(exp3, env), scm_void);
    REQUIRE_EQ(scm_env_lookup_var(env, a), scm_true);

    scm_object *exp4 = scm_list(4, sym_if, scm_false, scm_list(3, sym_set, a, scm_false), scm_chars['b']);
    REQUIRE_EQ(scm_eval(exp4, env), scm_chars['b']);
    REQUIRE_EQ(scm_env_lookup_var(env, a), scm_true);

    scm_object_free(exp1);
    scm_object_free(exp2);
    scm_object_free(exp3);
    scm_object_free(exp4);
}

TEST(eval, begin) {
    TEST_INIT();

    scm_object *env = scm_global_env();
    scm_object *a = SYM(a);
    env = scm_env_extend(env, scm_list(1, a), scm_list(1, scm_true));

    /* return the value of last expression */
    scm_object *exp1 = scm_list(2, sym_begin, scm_true);
    REQUIRE_EQ(scm_eval(exp1, env), scm_true);
    /* other expressions also get evaluated */
    scm_object *exp2 = scm_list(3, sym_begin, scm_list(3, sym_set, a, scm_false), scm_true);
    REQUIRE_EQ(scm_eval(exp2, env), scm_true);
    REQUIRE_EQ(scm_env_lookup_var(env, a), scm_false);


    scm_object_free(exp1);
    scm_object_free(exp2);
}

TEST(eval, quasiquote) {
    TEST_INIT();

    scm_object *env = scm_global_env();
    scm_object *a = SYM(a);
    scm_object *b = SYM(b);
    scm_object *c = SYM(c);
    scm_object *d = SYM(d);
    scm_object *e = SYM(e);
    env = scm_env_extend(env, scm_list(5, a, b, c, d, e),
                         scm_list(5, scm_true, scm_cons(scm_chars['a'], scm_chars['b']),
                                  scm_null, scm_list(1, scm_chars['a']),
                                  scm_list(2, scm_chars['a'], scm_chars['b'])));

    /* exp is not a list or vector, returns directly without evaluation */
    scm_object *exp1 = scm_list(2, sym_quasiquote, scm_true);
    REQUIRE_EQ(scm_eval(exp1, env), scm_true);
    scm_object *exp2 = scm_list(2, sym_quasiquote, a);
    REQUIRE_EQ(scm_eval(exp2, env), a);

    /* unquotation */
    scm_object *exp3 = scm_list(2, sym_quasiquote, scm_list(2, sym_unquote, a));
    REQUIRE_EQ(scm_eval(exp3, env), scm_true);

    /* list */
    scm_object *exp4 = scm_list(2, sym_quasiquote,
                                scm_list(3, a, scm_list(2, sym_unquote, a),
                                         scm_list(2, a, scm_list(2, sym_unquote, a))));
    REQUIRE_OBJ_EQUAL(scm_eval(exp4, env), scm_list(3, a, scm_true, scm_list(2, a, scm_true)));

    /* vector */
    scm_object *exp5 = scm_list(2, sym_quasiquote,
                                scm_vector_new(3, a, scm_list(2, sym_unquote, a),
                                               scm_vector_new(2, a, scm_list(2, sym_unquote, a))));
    REQUIRE_OBJ_EQUAL(scm_eval(exp5, env), scm_vector_new(3, a, scm_true, scm_vector_new(2, a, scm_true)));

    /* unquote_splicing in list */
    scm_object *exp6 = scm_list(2, sym_quasiquote, scm_list(2, scm_list(2, sym_unquote_splicing, c), a));
    REQUIRE_OBJ_EQUAL(scm_eval(exp6, env), scm_list(1, a));
    scm_object *exp7 = scm_list(2, sym_quasiquote, scm_list(2, scm_list(2, sym_unquote_splicing, d), a));
    REQUIRE_OBJ_EQUAL(scm_eval(exp7, env), scm_list(2, scm_chars['a'], a));
    scm_object *exp8 = scm_list(2, sym_quasiquote, scm_list(2, scm_list(2, sym_unquote_splicing, e), a));
    REQUIRE_OBJ_EQUAL(scm_eval(exp8, env), scm_list(3, scm_chars['a'], scm_chars['b'], a));

    scm_object *exp9 = scm_list(2, sym_quasiquote, scm_list(2, a, scm_list(2, sym_unquote_splicing, c)));
    REQUIRE_OBJ_EQUAL(scm_eval(exp9, env), scm_list(1, a));
    scm_object *exp10 = scm_list(2, sym_quasiquote, scm_list(2, a, scm_list(2, sym_unquote_splicing, d)));
    REQUIRE_OBJ_EQUAL(scm_eval(exp10, env), scm_list(2, a, scm_chars['a']));
    scm_object *exp11 = scm_list(2, sym_quasiquote, scm_list(2, a, scm_list(2, sym_unquote_splicing, e)));
    REQUIRE_OBJ_EQUAL(scm_eval(exp11, env), scm_list(3, a, scm_chars['a'], scm_chars['b']));

    /* unquote_splicing in improper list, normal for the elements except the last cdr */
    scm_object *exp12 = scm_list(2, sym_quasiquote, scm_cons(scm_list(2, sym_unquote_splicing, c), a));
    REQUIRE_OBJ_EQUAL(scm_eval(exp12, env), a);
    scm_object *exp13 = scm_list(2, sym_quasiquote, scm_cons(scm_list(2, sym_unquote_splicing, d), a));
    REQUIRE_OBJ_EQUAL(scm_eval(exp13, env), scm_cons(scm_chars['a'], a));
    scm_object *exp14 = scm_list(2, sym_quasiquote, scm_cons(scm_list(2, sym_unquote_splicing, e), a));
    REQUIRE_OBJ_EQUAL(scm_eval(exp14, env), scm_cons(scm_chars['a'], scm_cons(scm_chars['b'], a)));

    /* unquote_splicing can't be in the last cdr of improper list */
    scm_object *exp15 = scm_list(2, sym_quasiquote, scm_cons(a, scm_list(2, sym_unquote_splicing, c)));
    REQUIRE_EXC("unquote-splicing: not in context of sequence in", scm_eval(exp15, env));
    scm_object *exp16 = scm_list(2, sym_quasiquote, scm_cons(a, scm_list(2, sym_unquote_splicing, d)));
    REQUIRE_EXC("unquote-splicing: not in context of sequence in", scm_eval(exp16, env));
    scm_object *exp17 = scm_list(2, sym_quasiquote, scm_cons(a, scm_list(2, sym_unquote_splicing, e)));
    REQUIRE_EXC("unquote-splicing: not in context of sequence in", scm_eval(exp17, env));

    /* in nested list */
    scm_object *exp18 = scm_list(2, sym_quasiquote, scm_list(1, scm_list(1, scm_list(2, sym_unquote_splicing, e))));
    REQUIRE_OBJ_EQUAL(scm_eval(exp18, env), scm_list(1, scm_list(2, scm_chars['a'], scm_chars['b'])));

    /* unquote_splicing in vector */
    scm_object *exp19 = scm_list(2, sym_quasiquote, scm_vector_new(2, scm_list(2, sym_unquote_splicing, c), a));
    REQUIRE_OBJ_EQUAL(scm_eval(exp19, env), scm_vector_new(1, a));
    scm_object *exp20 = scm_list(2, sym_quasiquote, scm_vector_new(2, scm_list(2, sym_unquote_splicing, d), a));
    REQUIRE_OBJ_EQUAL(scm_eval(exp20, env), scm_vector_new(2, scm_chars['a'], a));
    scm_object *exp21 = scm_list(2, sym_quasiquote, scm_vector_new(2, scm_list(2, sym_unquote_splicing, e), a));
    REQUIRE_OBJ_EQUAL(scm_eval(exp21, env), scm_vector_new(3, scm_chars['a'], scm_chars['b'], a));

    scm_object *exp22 = scm_list(2, sym_quasiquote, scm_vector_new(2, a, scm_list(2, sym_unquote_splicing, c)));
    REQUIRE_OBJ_EQUAL(scm_eval(exp22, env), scm_vector_new(1, a));
    scm_object *exp23 = scm_list(2, sym_quasiquote, scm_vector_new(2, a, scm_list(2, sym_unquote_splicing, d)));
    REQUIRE_OBJ_EQUAL(scm_eval(exp23, env), scm_vector_new(2, a, scm_chars['a']));
    scm_object *exp24 = scm_list(2, sym_quasiquote, scm_vector_new(2, a, scm_list(2, sym_unquote_splicing, e)));
    REQUIRE_OBJ_EQUAL(scm_eval(exp24, env), scm_vector_new(3, a, scm_chars['a'], scm_chars['b']));

    /* in nested vector */
    scm_object *exp25 = scm_list(2, sym_quasiquote, scm_vector_new(1, scm_vector_new(1, scm_list(2, sym_unquote_splicing, e))));
    REQUIRE_OBJ_EQUAL(scm_eval(exp25, env), scm_vector_new(1, scm_vector_new(2, scm_chars['a'], scm_chars['b'])));

    /* unquote-splicing not in sequence */
    scm_object *exp26 = scm_list(2, sym_quasiquote, scm_list(2, sym_unquote_splicing, d));
    REQUIRE_EXC("unquote-splicing: not in context of sequence in", scm_eval(exp26, env));

    /* unquote-splicing not returning a list */
    scm_object *exp27 = scm_list(2, sym_quasiquote, scm_list(1, scm_list(2, sym_unquote_splicing, a)));
    REQUIRE_EXC("unquote-splicing: not returning a list in", scm_eval(exp27, env));
    scm_object *exp28 = scm_list(2, sym_quasiquote, scm_list(1, scm_list(2, sym_unquote_splicing, b)));
    REQUIRE_EXC("unquote-splicing: not returning a list in", scm_eval(exp28, env));

    /* unquote/unquote-splicing not in quasiquote */
    scm_object *exp29 = scm_list(2, sym_unquote, a);
    REQUIRE_EXC("unquote: not in quasiquote in", scm_eval(exp29, env));
    scm_object *exp30 = scm_list(2, sym_unquote_splicing, a);
    REQUIRE_EXC("unquote-splicing: not in quasiquote in", scm_eval(exp30, env));

    /* bad syntax: too short/long */
    scm_object *exp31 = scm_list(2, sym_quasiquote, scm_list(1, sym_unquote));
    REQUIRE_EXC("unquote: bad syntax in", scm_eval(exp31, env));
    scm_object *exp32 = scm_list(2, sym_quasiquote, scm_list(1, sym_unquote_splicing));
    REQUIRE_EXC("unquote-splicing: bad syntax in", scm_eval(exp32, env));
    scm_object *exp33 = scm_list(2, sym_quasiquote, scm_list(3, sym_unquote, a, a));
    REQUIRE_EXC("unquote: bad syntax in", scm_eval(exp33, env));
    scm_object *exp34 = scm_list(2, sym_quasiquote, scm_list(3, sym_unquote_splicing, d, d));
    REQUIRE_EXC("unquote-splicing: bad syntax in", scm_eval(exp34, env));

    /* unquote/unquote-splicing in the middle of list */
    scm_object *exp35 = scm_list(2, sym_quasiquote, scm_list(4, a, sym_unquote, a, a));
    REQUIRE_EXC("unquote: bad syntax in", scm_eval(exp35, env));
    scm_object *exp36 = scm_list(2, sym_quasiquote, scm_list(4, a, sym_unquote_splicing, d, a));
    REQUIRE_EXC("unquote-splicing: bad syntax in", scm_eval(exp36, env));

    /* unquote in the last cdr */
    scm_object *exp37 = scm_list(2, sym_quasiquote, scm_list(3, a, sym_unquote, a));
    REQUIRE_OBJ_EQUAL(scm_eval(exp37, env), scm_cons(a, scm_true));

    /* unquote-splicing in the last cdr */
    scm_object *exp38 = scm_list(2, sym_quasiquote, scm_list(3, a, sym_unquote_splicing, d));
    REQUIRE_EXC("unquote-splicing: not in context of sequence in", scm_eval(exp38, env));

    /* invalid context within quasiquote */
    scm_object *exp39 = scm_list(2, sym_quasiquote, scm_cons(a, sym_unquote));
    REQUIRE_EXC("unquote: invalid context within quasiquote in", scm_eval(exp39, env));
    scm_object *exp40 = scm_list(2, sym_quasiquote, scm_cons(a, sym_unquote_splicing));
    REQUIRE_EXC("unquote-splicing: invalid context within quasiquote in", scm_eval(exp40, env));
    scm_object *exp41 = scm_list(2, sym_quasiquote, scm_vector_new(1, sym_unquote));
    REQUIRE_EXC("unquote: invalid context within quasiquote in", scm_eval(exp41, env));
    scm_object *exp42 = scm_list(2, sym_quasiquote, scm_vector_new(1, sym_unquote_splicing));
    REQUIRE_EXC("unquote-splicing: invalid context within quasiquote in", scm_eval(exp42, env));
}

TEST(eval, application) {
    TEST_INIT();

    scm_object *env = scm_global_env();
    scm_object *a = SYM(a);
    env = scm_env_extend(env, scm_list(1, a), scm_list(1, scm_cons(scm_true, scm_false)));

    scm_object *exp1 = scm_list(2, SYM(car), SYM(a));
    REQUIRE_EQ(scm_eval(exp1, env), scm_true);
    scm_object *exp2 = scm_list(2, SYM(cdr), SYM(a));
    REQUIRE_EQ(scm_eval(exp2, env), scm_false);

    scm_object *exp3 = scm_list(2, scm_true, scm_true);
    REQUIRE_EXC("#%app: not a procedure;\nexpected a procedure that can be applied to arguments\ngiven: ",
                scm_eval(exp3, env));

    scm_object *exp4 = scm_null;
    REQUIRE_EXC("#%app: missing procedure expression in", scm_eval(exp4, env));

    scm_object *exp5 = scm_cons(scm_true, scm_true);
    REQUIRE_EXC("#%app: bad syntax in", scm_eval(exp5, env));


    /* the number of arguments 'n' is correctly passed */
    scm_object *exp6 = scm_list(3, SYM(vector), scm_true, scm_false);
    scm_object *vec = scm_eval(exp6, env);
    REQUIRE_EQ(vec->type, scm_type_vector);
    REQUIRE_EQ(scm_vector_length(vec), 2);
}
