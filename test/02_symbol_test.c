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
            CHECK_EQ(scm_object_eq(syms[i], syms[j]), i/2 == j/2);
            CHECK_EQ(scm_object_eqv(syms[i], syms[j]), i/2 == j/2);
            CHECK_EQ(scm_object_equal(syms[i], syms[j]), i/2 == j/2);
        }
        scm_object_free(syms[i]);
    }
}
