#include "test.h"

TAU_MAIN()

TEST(object, eof_true_false_dot_rparen) {
    int i;
    TEST_INIT();

    scm_object *objs[] = {scm_eof, scm_true, scm_false, scm_dot, scm_rparen};
    scm_type types[] = {scm_type_eof, scm_type_true, scm_type_false, scm_type_dot, scm_type_rparen};

    for (i = 0; i < (int)(sizeof(objs)/sizeof(scm_object *)); ++i) {
      CHECK_EQ(objs[i]->type, types[i], "i=%d", i);
    }
}

TEST(object, scm_chars) {
    int i;
    TEST_INIT();

    for (i = 0; i < 128; ++i) {
      CHECK_EQ(scm_chars[i]->type, scm_type_char);

      char c = scm_char_get_char(scm_chars[i]);
      CHECK_EQ(c, i, "i=%d", i);
    }
}

TEST(object, cant_free_static_objects) {
    int i;
    TEST_INIT();

    scm_object *objs[] = {scm_eof, scm_true, scm_false, scm_dot, scm_rparen};
    scm_type types[] = {scm_type_eof, scm_type_true, scm_type_false, scm_type_dot, scm_type_rparen};

    for (i = 0; i < (int)(sizeof(objs)/sizeof(scm_object *)); ++i) {
      scm_object_free(objs[i]);
      CHECK_EQ(objs[i]->type, types[i], "i=%d", i);
    }

    for (i = 0; i < 128; ++i) {
      scm_object_free(scm_chars[i]);
      CHECK_EQ(scm_chars[i]->type, scm_type_char, "i=%d", i);
    }
}

TEST(object, equivalence) {
    TEST_INIT();
    CHECK(!scm_object_eq(scm_eof, scm_true), "always return false for different types");
    CHECK(!scm_object_eqv(scm_eof, scm_true), "always return false for different types");
    CHECK(!scm_object_equal(scm_eof, scm_true), "always return false for different types");

    scm_object *trues[] = { scm_eof, scm_true, scm_false, };
    int n = sizeof(trues) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        CHECK_OBJ_EQ(trues[i], trues[i], "i=%d", i);
        CHECK_OBJ_EQV(trues[i], trues[i], "i=%d", i);
        CHECK_OBJ_EQUAL(trues[i], trues[i], "i=%d", i);
    }

    scm_object *chars[] = {scm_chars['a'], scm_chars['b']};

    for (int i = 0; i < 2; ++i) {
        for (int j = i; j < 2; ++j) {
            CHECK_EQ(scm_object_eq(chars[i], chars[j]), i == j, "i=%d,j=%d", i, j);
            CHECK_EQ(scm_object_eqv(chars[i], chars[j]), i == j, "i=%d,j=%d", i, j);
            CHECK_EQ(scm_object_equal(chars[i], chars[j]), i == j, "i=%d,j=%d", i, j);
        }
    }
}
