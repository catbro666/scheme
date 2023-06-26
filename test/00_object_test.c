#include "../src/object.h"
#include "../src/char.h"
#include <tau/tau.h>

TAU_MAIN()

void init() {
    int res;
    res = scm_object_env_init();
    REQUIRE(!res, "scm_object_env_init");
    res = scm_char_env_init();
    REQUIRE(!res, "scm_char_env_init");
}

TEST(object, eof_true_false_dot_rparen) {
    int i;
    init();

    scm_object *objs[] = {scm_eof, scm_true, scm_false, scm_dot, scm_rparen};
    scm_type types[] = {scm_type_eof, scm_type_true, scm_type_false, scm_type_dot, scm_type_rparen};

    for (i = 0; i < (int)(sizeof(objs)/sizeof(scm_object *)); ++i) {
      CHECK_EQ(objs[i]->type, types[i]);
    }
}

TEST(object, scm_chars) {
    int i;
    init();

    for (i = 0; i < 128; ++i) {
      CHECK_EQ(scm_chars[i]->type, scm_type_char);

      char c = scm_char_get_char(scm_chars[i]);
      CHECK_EQ(c, i);
    }
}

TEST(object, cant_free_static_objects) {
    int i;
    init();

    scm_object *objs[] = {scm_eof, scm_true, scm_false, scm_dot, scm_rparen};
    scm_type types[] = {scm_type_eof, scm_type_true, scm_type_false, scm_type_dot, scm_type_rparen};

    for (i = 0; i < (int)(sizeof(objs)/sizeof(scm_object *)); ++i) {
      scm_object_free(objs[i]);
      CHECK_EQ(objs[i]->type, types[i]);
    }

    for (i = 0; i < 128; ++i) {
      scm_object_free(scm_chars[i]);
      CHECK_EQ(scm_chars[i]->type, scm_type_char);
    }
}
