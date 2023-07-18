#include "test.h"

TAU_MAIN()

TEST(proc, check_arity) {
    TEST_INIT();

    /* fixed number of parameters */
    scm_object *p0 = scm_primitive_new("p0", NULL, 0, 0, NULL);
    REQUIRE_NOEXC(scm_procedure_check_arity(p0, 0));
    REQUIRE_EXC("p0: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 0\ngiven: 1", scm_procedure_check_arity(p0, 1));
    REQUIRE_EXC("p0: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 0\ngiven: 2", scm_procedure_check_arity(p0, 2));

    scm_object *p1 = scm_primitive_new("p1", NULL, 1, 1, NULL);
    REQUIRE_NOEXC(scm_procedure_check_arity(p1, 1));
    REQUIRE_EXC("p1: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 1\ngiven: 0", scm_procedure_check_arity(p1, 0));
    REQUIRE_EXC("p1: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 1\ngiven: 2", scm_procedure_check_arity(p1, 2));

    scm_object *p2 = scm_primitive_new("p2", NULL, 2, 2, NULL);
    REQUIRE_NOEXC(scm_procedure_check_arity(p2, 2));
    REQUIRE_EXC("p2: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 2\ngiven: 0", scm_procedure_check_arity(p2, 0));
    REQUIRE_EXC("p2: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 2\ngiven: 1", scm_procedure_check_arity(p2, 1));
    REQUIRE_EXC("p2: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 2\ngiven: 3", scm_procedure_check_arity(p2, 3));

    scm_object *p3 = scm_primitive_new("p3", NULL, 3, 3, NULL);
    REQUIRE_NOEXC(scm_procedure_check_arity(p3, 3));
    REQUIRE_EXC("p3: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 3\ngiven: 0", scm_procedure_check_arity(p3, 0));
    REQUIRE_EXC("p3: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 3\ngiven: 1", scm_procedure_check_arity(p3, 1));
    REQUIRE_EXC("p3: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 3\ngiven: 2", scm_procedure_check_arity(p3, 2));
    REQUIRE_EXC("p3: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 3\ngiven: 4", scm_procedure_check_arity(p3, 4));

    /* variadic parameters */
    scm_object *p0n = scm_primitive_new("p0n", NULL, 0, -1, NULL);
    REQUIRE_NOEXC(scm_procedure_check_arity(p0n, 0));
    REQUIRE_NOEXC(scm_procedure_check_arity(p0n, 1));
    REQUIRE_NOEXC(scm_procedure_check_arity(p0n, 2));
    REQUIRE_NOEXC(scm_procedure_check_arity(p0n, 3));

    scm_object *p1n = scm_primitive_new("p1n", NULL, 1, 2, NULL);
    REQUIRE_NOEXC(scm_procedure_check_arity(p1n, 1));
    REQUIRE_NOEXC(scm_procedure_check_arity(p1n, 2));
    REQUIRE_EXC("p1n: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: [1, 2]\ngiven: 0", scm_procedure_check_arity(p1n, 0));
    REQUIRE_EXC("p1n: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: [1, 2]\ngiven: 3", scm_procedure_check_arity(p1n, 3));

    scm_object *p2n = scm_primitive_new("p2n", NULL, 2, 4, NULL);
    REQUIRE_NOEXC(scm_procedure_check_arity(p2n, 2));
    REQUIRE_NOEXC(scm_procedure_check_arity(p2n, 3));
    REQUIRE_NOEXC(scm_procedure_check_arity(p2n, 4));
    REQUIRE_EXC("p2n: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: [2, 4]\ngiven: 0", scm_procedure_check_arity(p2n, 0));
    REQUIRE_EXC("p2n: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: [2, 4]\ngiven: 1", scm_procedure_check_arity(p2n, 1));
    REQUIRE_EXC("p2n: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: [2, 4]\ngiven: 5", scm_procedure_check_arity(p2n, 5));

    scm_object *p3n = scm_primitive_new("p3n", NULL, 3, -1, NULL);
    REQUIRE_NOEXC(scm_procedure_check_arity(p3n, 3));
    REQUIRE_NOEXC(scm_procedure_check_arity(p3n, 4));
    REQUIRE_EXC("p3n: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: at least 3\ngiven: 0", scm_procedure_check_arity(p3n, 0));
    REQUIRE_EXC("p3n: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: at least 3\ngiven: 1", scm_procedure_check_arity(p3n, 1));
    REQUIRE_EXC("p3n: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: at least 3\ngiven: 2", scm_procedure_check_arity(p3n, 2));

    /* compound */
    scm_object *comp1 = scm_compound_new(scm_list(1, SYM(a)), scm_list(1, scm_true), NULL);
    REQUIRE_NOEXC(scm_procedure_check_arity(comp1, 1));
    REQUIRE_EXC("arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 1\ngiven: 0", scm_procedure_check_arity(comp1, 0));
    REQUIRE_EXC("arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: 1\ngiven: 2", scm_procedure_check_arity(comp1, 2));

    scm_object *comp2 = scm_compound_new(scm_cons(SYM(a), SYM(b)), scm_list(1, scm_true), NULL);
    REQUIRE_NOEXC(scm_procedure_check_arity(comp2, 1));
    REQUIRE_NOEXC(scm_procedure_check_arity(comp2, 2));
    REQUIRE_NOEXC(scm_procedure_check_arity(comp2, 3));
    REQUIRE_EXC("arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: at least 1\ngiven: 0", scm_procedure_check_arity(comp2, 0));
    scm_compound_set_name(comp2, "comp2");
    REQUIRE_EXC("comp2: arity mismatch;\nthe expected number of arguments does not "
                "match the given number\nexpected: at least 1\ngiven: 0", scm_procedure_check_arity(comp2, 0));

}

TEST(proc, check_contract) {
    TEST_INIT();

    scm_object *p1 = scm_primitive_new("p1", NULL, 3, 3, pred_boolean);
    REQUIRE_NOEXC(scm_procedure_check_contract(p1, scm_list(3, scm_true, scm_true, scm_true)));
    REQUIRE_EXC("p1: contract violation by argument #1\nexpected: boolean?\ngiven: ()",
                scm_procedure_check_contract(p1, scm_list(3, scm_null, scm_true, scm_true)));
    REQUIRE_EXC("p1: contract violation by argument #2\nexpected: boolean?\ngiven: ()",
                scm_procedure_check_contract(p1, scm_list(3, scm_true, scm_null, scm_true)));
    REQUIRE_EXC("p1: contract violation by argument #3\nexpected: boolean?\ngiven: ()",
                scm_procedure_check_contract(p1, scm_list(3, scm_true, scm_true, scm_null)));

    scm_object *p2 = scm_primitive_new("p2", NULL, 3, 3, scm_cons(pred_boolean, pred_char));
    REQUIRE_NOEXC(scm_procedure_check_contract(p2, scm_list(3, scm_true, scm_chars['a'], scm_chars['b'])));
    REQUIRE_EXC("p2: contract violation by argument #1\nexpected: boolean?\ngiven: ()",
                scm_procedure_check_contract(p2, scm_list(3, scm_null, scm_chars['a'], scm_chars['b'])));
    REQUIRE_EXC("p2: contract violation by argument #2\nexpected: char?\ngiven: ()",
                scm_procedure_check_contract(p2, scm_list(3, scm_true, scm_null, scm_chars['b'])));
    REQUIRE_EXC("p2: contract violation by argument #3\nexpected: char?\ngiven: ()",
                scm_procedure_check_contract(p2, scm_list(3, scm_true, scm_chars['a'], scm_null)));

    scm_object *p3 = scm_primitive_new("p3", NULL, 3, 3, scm_list(2, pred_boolean, pred_char));
    REQUIRE_NOEXC(scm_procedure_check_contract(p3, scm_list(3, scm_true, scm_chars['a'], scm_true)));
    REQUIRE_NOEXC(scm_procedure_check_contract(p3, scm_list(3, scm_true, scm_chars['a'], scm_chars['b'])));
    REQUIRE_NOEXC(scm_procedure_check_contract(p3, scm_list(3, scm_true, scm_chars['a'], scm_null)));
    REQUIRE_EXC("p3: contract violation by argument #1\nexpected: boolean?\ngiven: ()",
                scm_procedure_check_contract(p3, scm_list(3, scm_null, scm_chars['a'], scm_null)));
    REQUIRE_EXC("p3: contract violation by argument #2\nexpected: char?\ngiven: ()",
                scm_procedure_check_contract(p3, scm_list(3, scm_true, scm_null, scm_null)));
}

static scm_object *scm_0() {
    return scm_null;
}
define_primitive_0(0);

static scm_object *scm_1(scm_object *p1) {
    return scm_list(1, p1);
}
define_primitive_1(1);

static scm_object *scm_2(scm_object *p1, scm_object *p2) {
    return scm_list(2, p1, p2);
}
define_primitive_2(2);

static scm_object *scm_3(scm_object *p1, scm_object *p2, scm_object *p3) {
    return scm_list(3, p1, p2, p3);
}
define_primitive_3(3);

static scm_object *scm_0n(int n, scm_object *l) {
    return scm_cons(INTEGER(n), l);
}
define_primitive_0n(0n);

static scm_object *scm_1n(int n, scm_object *p1, scm_object *l) {
    return scm_cons(INTEGER(n), scm_cons(p1, l));
}
define_primitive_1n(1n);

static scm_object *scm_2n(int n, scm_object *p1, scm_object *p2, scm_object *l) {
    return scm_cons(INTEGER(n), scm_cons(p1, scm_cons(p2, l)));
}
define_primitive_2n(2n);

static scm_object *scm_3n(int n, scm_object *p1, scm_object *p2, scm_object *p3, scm_object *l) {
    return scm_cons(INTEGER(n), scm_cons(p1, scm_cons(p2, scm_cons(p3, l))));
}
define_primitive_3n(3n);

TEST(proc, primitive_apply) {
    scm_object *procs[] = {
        scm_primitive_new("0", prim_0, 0, 0, NULL),
        scm_primitive_new("1", prim_1, 1, 1, NULL),
        scm_primitive_new("2", prim_2, 2, 2, NULL),
        scm_primitive_new("3", prim_3, 3, 3, NULL),
    };
    scm_object *args[] = {
        scm_null,
        scm_list(1, scm_true),
        scm_list(2, scm_true, scm_false),
        scm_list(3, scm_true, scm_false, scm_null),
    };

    int n = sizeof(procs) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        REQUIRE_OBJ_EQUAL(scm_primitive_apply(procs[i], i, args[i]), args[i], "i=%d", i);
    }

    scm_object *procs2[] = {
        scm_primitive_new("0n", prim_0n, 0, -1, NULL),
        scm_primitive_new("1n", prim_1n, 1, -1, NULL),
        scm_primitive_new("2n", prim_2n, 2, -1, NULL),
        scm_primitive_new("3n", prim_3n, 3, -1, NULL),
    };

    scm_object *args2[] = {
        scm_null,
        scm_list(1, scm_true),
        scm_list(2, scm_true, scm_false),
        scm_list(3, scm_true, scm_false, scm_null),
        scm_list(4, scm_true, scm_false, scm_null, scm_void),
    };

    n = sizeof(procs2) / sizeof(scm_object *);
    scm_object *o;
    for (int i = 0; i < n; ++i) {
        o = scm_primitive_apply(procs2[i], i, args2[i]);
        REQUIRE_OBJ_EQUAL(scm_car(o), INTEGER(i), "i=%d", i);
        REQUIRE_OBJ_EQUAL(scm_cdr(o), args2[i], "i=%d", i);

        o = scm_primitive_apply(procs2[i], i+1, args2[i+1]);
        REQUIRE_OBJ_EQUAL(scm_car(o), INTEGER(i+1), "i=%d", i);
        REQUIRE_OBJ_EQUAL(scm_cdr(o), args2[i+1], "i=%d", i);
    }

}

TEST(proc, compound_apply) {
    scm_object *env = scm_global_env();
    scm_object *a = SYM(a);
    scm_object *b = SYM(b);
    env = scm_env_extend(env, scm_list(2, a, b), scm_list(2, scm_false, scm_cons(scm_true, scm_false)));

    scm_object *x = SYM(x);
    scm_object *y = SYM(y);
    scm_object *set = SYM(set!);
    scm_object *cons = SYM(cons);
    scm_object *comp = scm_compound_new(scm_list(1, x),
                                        scm_list(2, scm_list(3, set, a, scm_true),
                                                 scm_list(3, cons, x, b)), env);
    scm_object *o;
    o = scm_compound_apply(comp, scm_list(1, scm_null)); 
    /* return the value of the last expression */
    REQUIRE_OBJ_EQUAL(o, scm_cons(scm_null, scm_cons(scm_true, scm_false)));
    /* the whole body gets evaluated */
    REQUIRE_OBJ_EQUAL(scm_env_lookup_var(env, a), scm_true);

    /* variadic parameter */
    scm_object *comp2 = scm_compound_new(scm_cons(x, y), scm_list(1, scm_list(3, cons, y, x)), env);
    o = scm_compound_apply(comp2, scm_list(3, scm_chars['a'], scm_chars['b'], scm_chars['c']));
    REQUIRE_OBJ_EQUAL(o, scm_cons(scm_list(2, scm_chars['b'], scm_chars['c']), scm_chars['a']));
}

//TEST(proc, equivalence) {
//}

