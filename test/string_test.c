#include "../src/string.h"
#include <tau/tau.h>
#include <string.h>

TAU_MAIN()

TEST(string, normal_string) {
    int res, i;
    res = scm_string_env_init();
    REQUIRE(!res, "scm_string_env_init");
    res = scm_string_env_init();
    REQUIRE(!res, "call scm_string_env_init the second time");

    char *str = malloc(4);
    strcpy(str, "ab1");
    scm_object *obj = scm_string_new(str, 3);
    REQUIRE(obj, "scm_string_new");

    scm_type type = scm_object_get_type(obj);
    CHECK_EQ(type, scm_type_string);

    void *data = (void *)scm_object_get_data(obj);
    CHECK(data);

    int len = scm_string_length(obj);
    CHECK_EQ(len, 3);

    scm_object *c;
    for (i = 0; i < 3; ++i) {
        c = scm_string_ref(obj, i);
        CHECK_EQ(c, scm_chars[i]);
    }
    for (i = -1; i < 4; i = i + 4) {
        c = scm_string_ref(obj, i);
        CHECK_EQ(c, NULL);
    }

    scm_object_free(obj);
}

TEST(string, not_specify_length) {
    int res, i;
    res = scm_string_env_init();
    REQUIRE(!res, "scm_string_env_init");
    res = scm_string_env_init();
    REQUIRE(!res, "call scm_string_env_init the second time");

    char *str = malloc(4);
    strcpy(str, "ab1");
    scm_object *obj = scm_string_new(str, -1);
    REQUIRE(obj, "scm_string_new");

    scm_type type = scm_object_get_type(obj);
    CHECK_EQ(type, scm_type_string);

    void *data = (void *)scm_object_get_data(obj);
    CHECK(data);

    int len = scm_string_length(obj);
    CHECK_EQ(len, 3);

    scm_object *c;
    for (i = 0; i < 3; ++i) {
        c = scm_string_ref(obj, i);
        CHECK_EQ(c, scm_chars[i]);
    }
    for (i = -1; i < 4; i = i + 4) {
        c = scm_string_ref(obj, i);
        CHECK_EQ(c, NULL);
    }

    scm_object_free(obj);
}

TEST(string, empty_string) {
    int res, i;
    res = scm_string_env_init();
    REQUIRE(!res, "scm_string_env_init");
    res = scm_string_env_init();
    REQUIRE(!res, "call scm_string_env_init the second time");

    scm_object *obj = scm_string_new(NULL, 0);
    REQUIRE(obj, "scm_string_new");

    scm_type type = scm_object_get_type(obj);
    CHECK_EQ(type, scm_type_string);

    void *data = (void *)scm_object_get_data(obj);
    CHECK(data);

    int len = scm_string_length(obj);
    CHECK_EQ(len, 0);

    scm_object *c;
    for (i = -1; i < 2; ++i) {
        c = scm_string_ref(obj, i);
        CHECK_EQ(c, NULL);
    }

    scm_object_free(obj);
}


