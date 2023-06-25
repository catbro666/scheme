#include "../src/char.h"
#include "../src/number.h"
#include "../src/vector.h"
#include <tau/tau.h>

TAU_MAIN()

void init() {
    int res;
    res = scm_char_env_init();
    REQUIRE(!res, "scm_char_env_init");
    res = scm_number_env_init();
    REQUIRE(!res, "scm_number_env_init");
    res = scm_vector_env_init();
    REQUIRE(!res, "scm_vector_env_init");
}

TEST(vector, new_ref) {
    scm_object *o = NULL;
    init();

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
    init();

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
    init();

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
    init();

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
