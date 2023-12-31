#include "test.h"

TAU_MAIN()

TEST(write, simple_datum) {
    char buf[1024] = {0};
    TEST_INIT();

    scm_object *oport = string_output_port_new(buf, 1024);
    REQUIRE(oport, "string_output_port_new");
    scm_object *iport = string_input_port_new(buf, 1024);
    REQUIRE(iport, "string_input_port_new");

    scm_object *objs[] = {
        scm_eof, scm_void, scm_null, scm_true, scm_false, iport, oport,
    };

    char *expected[] = {
        "#<eof-object>", "#<void>", "()", "#t", "#f", "#<input-port>", "#<output-port>",
    };

    int pos = 0;
    int n = sizeof(expected) / sizeof(char *);
    for (int i = 0; i < n; ++i) {
        size_t len = scm_write(oport, objs[i]);
        REQUIRE_EQ(len, strlen(expected[i]), "i=%d", i);
        REQUIRE_STREQ(buf + pos, expected[i], "i=%d", i);
        pos += len;
    }

    scm_object_free(iport);
    scm_object_free(oport);
}

TEST(write, char) {
    char buf[1024] = {0};
    TEST_INIT();

    char expected[4];
    scm_object *oport = string_output_port_new(buf, 1024);
    REQUIRE(oport, "string_output_port_new");

    int off = 0;
    int n = 128;
    for (int i = 0; i < n; ++i) {
        size_t len = scm_write(oport, scm_chars[i]);
        if (i == ' ') {
            REQUIRE_EQ(len, 7, "i=%d", i);
            REQUIRE_STREQ(buf + off, "#\\space", "i=%d", i);
        }
        else if (i == '\n') {
            REQUIRE_EQ(len, 9, "i=%d", i);
            REQUIRE_STREQ(buf + off, "#\\newline", "i=%d", i);
        }
        else {
            REQUIRE_EQ(len, 3, "i=%d", i);
            snprintf(expected, 4, "#\\%c", (char)i);
            REQUIRE_STREQ(buf + off, expected, "i=%d", i);
        }
        off += len;
    }

    scm_object_free(oport);
}

TEST(write, number) {
    char buf[1024] = {0};
    TEST_INIT();

    scm_object *oport = string_output_port_new(buf, 1024);
    REQUIRE(oport, "string_output_port_new");

    char *nums[] = {
        "10", "-66", "1.0", "-2e-100",
    };

    int pos = 0;
    int n = sizeof(nums) / sizeof(char *);
    for (int i = 0; i < n; ++i) {
        scm_object *o;
        if (i < 2) {
            o = scm_number_new_integer(nums[i], 10);
        }
        else {
            o = scm_number_new_float(nums[i]);
        }
        size_t len = scm_write(oport, o);
        REQUIRE_EQ(len, strlen(nums[i]), "i=%d", i);
        REQUIRE_STREQ(buf + pos, nums[i], "i=%d", i);
        pos += len;
        scm_object_free(o);
    }

    scm_object_free(oport);
}

TEST(write, string) {
    char buf[1024] = {0};
    TEST_INIT();

    scm_object *oport = string_output_port_new(buf, 1024);
    REQUIRE(oport, "string_output_port_new");

    char *str = "a1\\\"";
    char *expected = "\"a1\\\\\\\"\"";
    scm_object *o = scm_string_copy_new(str, -1);
    size_t len = scm_write(oport, o);
    REQUIRE_EQ(len, strlen(expected));
    REQUIRE_STREQ(buf, expected);

    scm_object_free(o);
    scm_object_free(oport);
}

TEST(write, symbol) {
    char buf[1024] = {0};
    TEST_INIT();

    scm_object *oport = string_output_port_new(buf, 1024);
    REQUIRE(oport, "string_output_port_new");

    char *str = "abc";
    scm_object *o = scm_symbol_new(str, -1);
    size_t len = scm_write(oport, o);
    REQUIRE_EQ(len, strlen(str));
    REQUIRE_STREQ(buf, str);

    scm_object_free(o);
    scm_object_free(oport);
}

TEST(write, pair) {
    char buf[1024] = {0};
    TEST_INIT();

    scm_object *oport = string_output_port_new(buf, 1024);
    REQUIRE(oport, "string_output_port_new");

    scm_object *objs[] = {
        scm_cons(scm_true, scm_false),
        scm_list(2, scm_true, scm_false),
        scm_list(2, scm_cons(scm_true, scm_false), scm_cons(scm_true, scm_false)),
    };

    char *expected[] = {
        "(#t . #f)", "(#t #f)", "((#t . #f) (#t . #f))",
    };

    int pos = 0;
    int n = sizeof(expected) / sizeof(char *);
    for (int i = 0; i < n; ++i) {
        size_t len = scm_write(oport, objs[i]);
        REQUIRE_EQ(len, strlen(expected[i]), "i=%d", i);
        REQUIRE_STREQ(buf + pos, expected[i], "i=%d", i);
        pos += len;
        scm_object_free(objs[i]);
    }

    scm_object_free(oport);
}

TEST(write, vector) {
    char buf[1024] = {0};
    TEST_INIT();

    scm_object *oport = string_output_port_new(buf, 1024);
    REQUIRE(oport, "string_output_port_new");

    scm_object *objs[] = {
        scm_empty_vector, scm_vector_new(2, scm_true, SYM(a)),
    };

    char *expected[] = {
        "#()", "#(#t a)",
    };

    int pos = 0;
    int n = sizeof(expected) / sizeof(char *);
    for (int i = 0; i < n; ++i) {
        size_t len = scm_write(oport, objs[i]);
        REQUIRE_EQ(len, strlen(expected[i]), "i=%d", i);
        REQUIRE_STREQ(buf + pos, expected[i], "i=%d", i);
        pos += len;
        scm_object_free(objs[i]);
    }

    scm_object_free(oport);
}
