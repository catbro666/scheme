#include "test.h"

TAU_MAIN()

TEST(string, normal_string) {
    int i;
    TEST_INIT();

    char *str = "ab1";
    scm_object *obj = scm_string_copy_new(str, 3);
    REQUIRE(obj, "scm_string_copy_new");

    CHECK_EQ(obj->type, scm_type_string);

    int len = scm_string_length(obj);
    CHECK_EQ(len, 3);

    for (i = 0; i < 3; ++i) {
        CHECK_NOEXC_EQ(scm_string_ref(obj, i), scm_chars[(int)str[i]]);
    }
    for (i = -1; i < 4; i = i + 4) {
        CHECK_EXC("string-ref: index is out of range", scm_string_ref(obj, i));
    }

    scm_object_free(obj);
}

TEST(string, not_specify_length) {
    TEST_INIT();

    char *str = "ab1";
    scm_object *obj = scm_string_copy_new(str, -1);
    REQUIRE(obj, "scm_string_new");

    CHECK_EQ(obj->type, scm_type_string);

    int len = scm_string_length(obj);
    CHECK_EQ(len, 3);

    scm_object_free(obj);
}

TEST(string, empty_string) {
    TEST_INIT();

    scm_object *obj = scm_string_new(NULL, 0);
    REQUIRE(obj, "scm_string_new");

    CHECK_EQ(obj->type, scm_type_string);

    int len = scm_string_length(obj);
    CHECK_EQ(len, 0);

    int i;
    for (i = -1; i < 2; ++i) {
        CHECK_EXC("string-ref: index is out of range for empty string",
                  scm_string_ref(obj, i));
    }

    scm_object_free(obj);
}


TEST(string, equivalence) {
    TEST_INIT();
    scm_object *strs[] = {
        scm_string_copy_new("", 0),
        scm_string_copy_new("", 0),
        scm_string_copy_new("a", 1),
        scm_string_copy_new("a", 1),
        scm_string_copy_new("ab", 2),
        scm_string_copy_new("ab", 2),
        scm_string_copy_new("aa", 2),
        scm_string_copy_new("aa", 2),
    };
    int n = sizeof(strs) / sizeof(scm_object *);

    /* different strings */
    for (int i = 0; i < n; i += 2) {
        for (int j = i + 2; j < n; j += 2) {
            CHECK(!scm_object_eq(strs[i], strs[j]), "i=%d, j=%d", i, j);
            CHECK(!scm_object_eqv(strs[i], strs[j]), "i=%d, j=%d", i, j);
            CHECK(!scm_object_equal(strs[i], strs[j]), "i=%d, j=%d", i, j);
        }
    }

    /* strings that have the same content */
    for (int i = 0; i < n; i += 2) {
        CHECK_EQ(scm_object_eq(strs[i], strs[i+1]), i == 0); /* only empty string */
        CHECK_EQ(scm_object_eqv(strs[i], strs[i+1]), i == 0); /* only empty string */
        CHECK_EQ(scm_object_equal(strs[i], strs[i+1]), 1);
    }

    /* the same string */
    for (int i = 0; i < n; ++i) {
        CHECK(scm_object_eq(strs[i], strs[i]));
        CHECK(scm_object_eqv(strs[i], strs[i]));
        CHECK(scm_object_equal(strs[i], strs[i]));
        scm_object_free(strs[i]);
    }
}
