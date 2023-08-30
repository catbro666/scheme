#include "test.h"

TAU_MAIN()

TEST(symbol, new) {
    TEST_INIT();

    scm_object *obj = scm_symbol_new("ab1", 3);
    REQUIRE(obj, "scm_symbol_new");

    REQUIRE_EQ(obj->type, scm_type_identifier);

    char *res = scm_symbol_get_string(obj);

    REQUIRE_STREQ(res, "ab1");

    scm_object_free(obj);
}

TEST(esymbol, new) {
    TEST_INIT();
    scm_object *env = scm_global_env();

    scm_object *obj = scm_symbol_new("ab1", 3);
    REQUIRE(obj, "scm_symbol_new");

    scm_object *obj2 = scm_esymbol_new(obj, 1, env);
    REQUIRE_EQ(obj2->type, scm_type_eidentifier);

    REQUIRE_OBJ_EQ(scm_esymbol_get_symbol(obj2), obj);
    REQUIRE_EQ(scm_esymbol_get_uid(obj2), 1);
    REQUIRE_OBJ_EQ(scm_esymbol_get_env(obj2), env);
    REQUIRE_STREQ(scm_esymbol_get_string(obj2), "ab1");

    scm_object_free(obj);
    scm_object_free(obj2);
}

TEST(symbol, equivalence) {
    TEST_INIT();

    scm_object *syms[] = {
        scm_symbol_new("a", 1),
        scm_symbol_new("a", 1),
        scm_symbol_new("b", 1),
        scm_symbol_new("b", 1),
    };

    int n = sizeof(syms) / sizeof(scm_object *);

    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            CHECK_EQ(scm_eq(syms[i], syms[j]), i/2 == j/2, "i=%d,j=%d", i, j);
            CHECK_EQ(scm_eqv(syms[i], syms[j]), i/2 == j/2, "i=%d,j=%d", i, j);
            CHECK_EQ(scm_equal(syms[i], syms[j]), i/2 == j/2, "i=%d,j=%d", i, j);
        }
        scm_object_free(syms[i]);
    }
}
