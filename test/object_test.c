#include "../src/object.h"
#include <tau/tau.h>

TAU_MAIN()

TEST(object, eof_true_false) {
    int res, i;
    res = scm_object_env_init();
    REQUIRE(!res, "scm_object_env_init");
    res = scm_object_env_init();
    REQUIRE(!res, "call scm_object_env_init the second time");

    scm_object *objs[] = {scm_eof, scm_true, scm_false};
    scm_type types[] = {scm_type_eof, scm_type_true, scm_type_false};

    for (i = 0; i < (int)(sizeof(objs)/sizeof(scm_object *)); ++i) {
      scm_type type = scm_object_get_type(objs[i]);
      CHECK_EQ(type, types[i]);

      void *data = (void *)scm_object_get_data(objs[i]);
      CHECK(!data);
    }
}

TEST(object, scm_chars) {
    int res, i;
    res = scm_object_env_init();
    REQUIRE(!res, "scm_object_env_init");
    res = scm_object_env_init();
    REQUIRE(!res, "call scm_object_env_init the second time");

    for (i = 0; i < 128; ++i) {
      scm_type type = scm_object_get_type(scm_chars[i]);
      CHECK_EQ(type, scm_type_char);

      int data = (int)scm_object_get_data(scm_chars[i]);
      CHECK_EQ(data, i);
    }
}

TEST(object, cant_free_static_objects) {
    int res, i;
    res = scm_object_env_init();
    REQUIRE(!res, "scm_object_env_init");

    scm_object *objs[] = {scm_eof, scm_true, scm_false};
    scm_type types[] = {scm_type_eof, scm_type_true, scm_type_false};

    for (i = 0; i < (int)(sizeof(objs)/sizeof(scm_object *)); ++i) {
      scm_object_free(objs[i]);
      scm_type type = scm_object_get_type(objs[i]);
      CHECK_EQ(type, types[i]);
    }

    for (i = 0; i < 128; ++i) {
      scm_object_free(scm_chars[i]);
      scm_type type = scm_object_get_type(scm_chars[i]);
      CHECK_EQ(type, scm_type_char);
    }
}
