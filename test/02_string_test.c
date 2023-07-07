#include "../src/char.h"
#include "../src/string.h"
#include <tau/tau.h>
#include <string.h>

TAU_MAIN()

void init() {
    int res;
    res = scm_char_env_init();
    REQUIRE(!res, "scm_char_env_init");
    res = scm_string_env_init();
    REQUIRE(!res, "scm_string_env_init");
}

TEST(string, normal_string) {
    int i;
    init();

    char *str = malloc(4);
    strcpy(str, "ab1");
    scm_object *obj = scm_string_new(str, 3);
    REQUIRE(obj, "scm_string_new");

    CHECK_EQ(obj->type, scm_type_string);

    int len = scm_string_length(obj);
    CHECK_EQ(len, 3);

    scm_object *c;
    for (i = 0; i < 3; ++i) {
        c = scm_string_ref(obj, i);
        CHECK_EQ(c, scm_chars[(int)str[i]]);
    }
    //for (i = -1; i < 4; i = i + 4) {
    //    c = scm_string_ref(obj, i);
    //    CHECK_EQ(c, NULL);
    //}

    scm_object_free(obj);
}

TEST(string, not_specify_length) {
    int i;
    init();

    char *str = "ab1";
    scm_object *obj = scm_string_copy_new(str, -1);
    REQUIRE(obj, "scm_string_new");

    CHECK_EQ(obj->type, scm_type_string);

    int len = scm_string_length(obj);
    CHECK_EQ(len, 3);

    scm_object *c;
    for (i = 0; i < 3; ++i) {
        c = scm_string_ref(obj, i);
        CHECK_EQ(c, scm_chars[(int)str[i]]);
    }
    //for (i = -1; i < 4; i = i + 4) {
    //    c = scm_string_ref(obj, i);
    //    CHECK_EQ(c, NULL);
    //}

    scm_object_free(obj);
}

TEST(string, empty_string) {
    init();

    scm_object *obj = scm_string_new(NULL, 0);
    REQUIRE(obj, "scm_string_new");

    CHECK_EQ(obj->type, scm_type_string);

    int len = scm_string_length(obj);
    CHECK_EQ(len, 0);

    //int i;
    //scm_object *c;
    //for (i = -1; i < 2; ++i) {
    //    c = scm_string_ref(obj, i);
    //    CHECK_EQ(c, NULL);
    //}

    scm_object_free(obj);
}


