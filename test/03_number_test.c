#include "../src/number.h"
#include <tau/tau.h>
#include <limits.h>

TAU_MAIN()

void init() {
    int res;
    res = scm_number_env_init();
    REQUIRE(!res, "scm_number_env_init");
}

TEST(number, positive_integers) {
    int i;
    scm_object *obj;
    char *str = NULL;
    init();

    const int n = 4;
    char *strs[n] = {"998", "ffe", "110", "776"};
    int radices[n] = {10, 16, 2, 8};
    for (i = 0; i < n; ++i) {
        obj = scm_number_new_integer(strs[i], radices[i]);
        REQUIRE(obj, "scm_number_new_integer");

        REQUIRE_EQ(obj->type, scm_type_integer);

        str = scm_number_to_string(obj, radices[i]);
        REQUIRE_STREQ(str, strs[i]);

        free(str);
        scm_object_free(obj);
    }
}

TEST(number, zero_negative_integers) {
    int i;
    scm_object *obj;
    char *str = NULL;
    init();

    const int n = 8;
    char *strs[n] = {"-998", "-ffe", "-110", "-776", "0", "0", "0", "0"};
    int radices[n] = {10, 16, 2, 8, 10, 16, 2, 8};
    for (i = 0; i < n; ++i) {
        obj = scm_number_new_integer(strs[i], radices[i]);
        REQUIRE(obj, "scm_number_new_integer");

        REQUIRE_EQ(obj->type, scm_type_integer);

        str = scm_number_to_string(obj, radices[i]);
        REQUIRE_STREQ(str, strs[i]);

        free(str);
        scm_object_free(obj);
    }
}

TEST(number, integer_max_min) {
    int i, j;
    scm_object *obj, *obj2;
    char *str = NULL, *str2 = NULL;
    init();

    char max[256], min[256];
    snprintf(max, 256, "%ld", LONG_MAX);
    snprintf(min, 256, "%ld", LONG_MIN);

    const int n = 2;
    char *strs[n] = {max, min};
    int radices[4] = {10, 16, 8, 2};
    for (i = 0; i < n; ++i) {
        for (j = 0; j < 4; ++j) {
            obj = scm_number_new_integer(strs[i], 10);
            REQUIRE(obj, "scm_number_new_integer");

            REQUIRE_EQ(obj->type, scm_type_integer);

            str = scm_number_to_string(obj, radices[j]);

            obj2 = scm_number_new_integer(str, radices[j]);
            REQUIRE(obj2, "scm_number_new_integer");

            REQUIRE_EQ(obj2->type, scm_type_integer);

            str2 = scm_number_to_string(obj2, 10);
            REQUIRE_STREQ(str2, strs[i]);

            free(str);
            free(str2);
            scm_object_free(obj);
            scm_object_free(obj2);
        }
    }
}

TEST(number, integer_out_of_range) {
    int i;
    scm_object *obj;
    init();

    const int n = 2;
    char *strs[n] = {"9223372036854775808", "-9223372036854775809"};
    int radices[n] = {10, 10};
    for (i = 0; i < n; ++i) {
        obj = scm_number_new_integer(strs[i], radices[i]);
        REQUIRE(!obj, "scm_number_new_integer");
    }
}

TEST(number, floats) {
    int i;
    scm_object *obj;
    char *str = NULL;
    init();

    const int n = 5;
    char *strs[n] = {"100.0", "2e-3", "-0.000003", "-0.4e6", "666"};
    char *ress[n] = {"100.0", "0.002", "-3e-06", "-400000.0", "666.0"};
    for (i = 0; i < n; ++i) {
        obj = scm_number_new_float(strs[i]);
        REQUIRE(obj, "scm_number_new_float");

        REQUIRE_EQ(obj->type, scm_type_float);

        str = scm_number_to_string(obj, 10);
        REQUIRE_STREQ(str, ress[i]);

        free(str);
        scm_object_free(obj);
    }
}

TEST(number, floats_out_of_range) {
    int i;
    scm_object *obj;
    init();

    const int n = 2;
    char *strs[n] = {"1e1000", "1e-1000"};
    for (i = 0; i < n; ++i) {
        obj = scm_number_new_float(strs[i]);
        REQUIRE(!obj, "scm_number_new_float");
    }
}

TEST(number, integer_from_float) {
    int i;
    scm_object *obj;
    char *str = NULL;
    init();

    const int n = 5;
    char *strs[n] = {"100.0", "2e-3", "-0.999", "-0.4e6", "666"};
    char *ress[n] = {"100", "0", "-1", "-400000", "666"};
    for (i = 0; i < n; ++i) {
        obj = scm_number_new_integer_from_float(strs[i]);
        REQUIRE(obj, "scm_number_new_integer_from_float");

        REQUIRE_EQ(obj->type, scm_type_integer);

        str = scm_number_to_string(obj, 10);
        REQUIRE_STREQ(str, ress[i]);

        free(str);
        scm_object_free(obj);
    }
}

TEST(number, float_from_integer) {
    int i;
    scm_object *obj;
    char *str = NULL;
    init();

    const int n = 4;
    char *strs[n] = {"10", "10", "-10", "-10"};
    int radices[n] = {10, 16, 2, 8};
    char *ress[n] = {"10.0", "16.0", "-2.0", "-8.0"};
    for (i = 0; i < n; ++i) {
        obj = scm_number_new_float_from_integer(strs[i], radices[i]);
        REQUIRE(obj, "scm_number_new_float_from_integer");

        REQUIRE_EQ(obj->type, scm_type_float);

        str = scm_number_to_string(obj, 10);
        REQUIRE_STREQ(str, ress[i]);

        free(str);
        scm_object_free(obj);
    }
}


