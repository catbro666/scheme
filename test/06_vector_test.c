#include "test.h"

TAU_MAIN()

TEST(vector, new_ref) {
    scm_object *o = NULL;
    TEST_INIT();

    scm_object *v0 = scm_vector_new(0);
    REQUIRE(v0, "scm_vector_new 0 elements");
    REQUIRE_EQ(v0->type, scm_type_vector);
    REQUIRE_EQ(scm_vector_length(v0), 0);
    o = scm_vector_ref(v0, 0);
    REQUIRE(!o, "invalid index");

    scm_object *v1 = scm_vector_new(1, scm_chars['a']);
    REQUIRE(v1, "scm_vector_new 1 elements");
    REQUIRE_EQ(v1->type, scm_type_vector);
    REQUIRE_EQ(scm_vector_length(v1), 1);
    o = scm_vector_ref(v1, 0);
    REQUIRE(o, "scm_vector_ref(v1, 0)");
    REQUIRE_EQ(o->type, scm_type_char);
    REQUIRE_EQ(scm_char_get_char(o), 'a');
    o = scm_vector_ref(v1, 1);
    REQUIRE(!o, "invalid index");

    scm_object_free(v0);
    scm_object_free(v1);
}

TEST(vector, fill) {
    scm_object *o;
    TEST_INIT();

    scm_object *v1 = scm_vector_new(2, scm_chars['a'], scm_chars['b']);
    REQUIRE(v1, "scm_vector_new");
    REQUIRE_EQ(v1->type, scm_type_vector);
    REQUIRE_EQ(scm_vector_length(v1), 2);

    scm_vector_fill(v1, scm_chars['c']);

    o = scm_vector_ref(v1, 0);
    REQUIRE(o, "scm_vector_ref(v1, 0)");
    REQUIRE_EQ(scm_char_get_char(o), 'c');

    o = scm_vector_ref(v1, 1);
    REQUIRE(o, "scm_vector_ref(v1, 1)");
    REQUIRE_EQ(scm_char_get_char(o), 'c');

    scm_object *v2 = scm_vector_new_fill(2, scm_chars['a']);
    REQUIRE(v2, "scm_vector_new_fill");
    REQUIRE_EQ(v2->type, scm_type_vector);
    REQUIRE_EQ(scm_vector_length(v2), 2);
    o = scm_vector_ref(v2, 0);
    REQUIRE(o, "scm_vector_ref(v2, 0)");
    REQUIRE_EQ(scm_char_get_char(o), 'a');

    o = scm_vector_ref(v2, 1);
    REQUIRE(o, "scm_vector_ref(v2, 1)");
    REQUIRE_EQ(scm_char_get_char(o), 'a');

    scm_object_free(v1);
    scm_object_free(v2);
}

TEST(vector, set) {
    scm_object *o;
    TEST_INIT();

    scm_object *v1 = scm_vector_new(2, scm_chars['a'], scm_chars['b']);
    REQUIRE(v1, "scm_vector_new");
    REQUIRE_EQ(v1->type, scm_type_vector);
    REQUIRE_EQ(scm_vector_length(v1), 2);

    scm_vector_set(v1, 0, scm_chars['c']);
    o = scm_vector_ref(v1, 0);
    REQUIRE(o, "scm_vector_ref(v1, 0)");
    REQUIRE_EQ(scm_char_get_char(o), 'c');

    scm_vector_set(v1, 1, scm_chars['d']);
    o = scm_vector_ref(v1, 1);
    REQUIRE(o, "scm_vector_ref(v1, 1)");
    REQUIRE_EQ(scm_char_get_char(o), 'd');

    REQUIRE(scm_vector_set(v1, -1, scm_chars['x']));
    REQUIRE(scm_vector_set(v1, 3, scm_chars['x']));

    scm_object_free(v1);
}

TEST(vector, insert) {
    scm_object *o;
    TEST_INIT();

    /* empty vector */
    scm_object *v0 = scm_vector_new(0);
    REQUIRE(v0, "scm_vector_new");
    REQUIRE_EQ(v0->type, scm_type_vector);
    REQUIRE_EQ(scm_vector_length(v0), 0);

    v0 = scm_vector_insert(v0, scm_chars['a']);
    REQUIRE(v0, "scm_vector_insert(v0, scm_chars['a'])");
    REQUIRE_EQ(scm_vector_length(v0), 1);

    o = scm_vector_ref(v0, 0);
    REQUIRE(o, "scm_vector_ref(v0, 0)");
    REQUIRE_EQ(scm_char_get_char(o), 'a');

    /* non-empty vector */
    scm_object *v1 = scm_vector_new(1, scm_chars['a']);
    REQUIRE(v1, "scm_vector_new");
    REQUIRE_EQ(v1->type, scm_type_vector);
    REQUIRE_EQ(scm_vector_length(v1), 1);

    v1 = scm_vector_insert(v1, scm_chars['b']);
    REQUIRE(v1, "scm_vector_insert(v1, scm_chars['b'])");
    REQUIRE_EQ(scm_vector_length(v1), 2);

    o = scm_vector_ref(v1, 0);
    REQUIRE(o, "scm_vector_ref(v1, 0)");
    REQUIRE_EQ(scm_char_get_char(o), 'a');

    o = scm_vector_ref(v1, 1);
    REQUIRE(o, "scm_vector_ref(v1, 1)");
    REQUIRE_EQ(scm_char_get_char(o), 'b');

    scm_object_free(v0);
    scm_object_free(v1);
}

TEST(vector, equivalence) {
    TEST_INIT();
    scm_object *vecs[] = {
        scm_empty_vector,
        scm_empty_vector,
        scm_vector_new(1, scm_true),
        scm_vector_new(1, scm_true),
        scm_vector_new(1, scm_false),
        scm_vector_new(1, scm_false),
        scm_vector_new(2, scm_true, scm_false),
        scm_vector_new(2, scm_true, scm_false),
        scm_vector_new(1, scm_vector_new(1, scm_true)),
        scm_vector_new(1, scm_vector_new(1, scm_true)),
    };

    int n = sizeof(vecs) / sizeof(scm_object *);
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            CHECK_EQ(scm_object_eq(vecs[i], vecs[j]), i == j || (i == 0 && j == 1), "i=%d,j=%d", i, j);
            CHECK_EQ(scm_object_eqv(vecs[i], vecs[j]), i == j || (i == 0 && j == 1), "i=%d,j=%d", i, j);
            CHECK_EQ(scm_object_equal(vecs[i], vecs[j]), i/2 == j/2, "i=%d,j=%d", i, j);
        }
        scm_object_free(vecs[i]);
    }
}
