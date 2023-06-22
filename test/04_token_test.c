#include "../src/char.h"
#include "../src/port.h"
#include "../src/string.h"
#include "../src/number.h"
#include "../src/token.h"
#include <tau/tau.h>

TAU_MAIN()

static void init() {
    int res;
    res = scm_object_env_init();
    REQUIRE(!res, "scm_object_env_init");
    res = scm_char_env_init();
    REQUIRE(!res, "scm_char_env_init");
    res = scm_port_env_init();
    REQUIRE(!res, "scm_port_env_init");
    res = scm_string_env_init();
    REQUIRE(!res, "scm_string_env_init");
    res = scm_number_env_init();
    REQUIRE(!res, "scm_number_env_init");
    res = scm_token_env_init();
    REQUIRE(!res, "scm_token_env_init");
    res = scm_token_env_init();
    REQUIRE(!res, "call scm_token_env_init the second time");
}

TEST(token, simple) {
    int i;
    init();
    scm_object *port = string_input_port_new("\'`()#(,,@#t#f. ", -1);
    REQUIRE(port, "string_input_port_new");

    scm_token *expected[] = 
        { scm_token_quote, scm_token_bquote, scm_token_lparen, scm_token_rparen,
          scm_token_sharp_lparen, scm_token_comma, scm_token_comma_at, scm_token_true,
          scm_token_false, scm_token_dot, scm_token_eof,
        };
    int n = sizeof(expected) / sizeof(scm_token *);

    scm_token *t;
    for (i = 0; i < n; ++i) {
        t = scm_token_read(port);
        CHECK_EQ(t, expected[i]); 
        scm_token_free(t);
    }
    scm_object_free(port);
}

TEST(token, char) {
    int i;
    init();
    scm_object *port = string_input_port_new("#\\a #\\A #\\\\ #\\  #\\( #\\) #\\# #\\' #\\` "
                                             "#\\, #\\@ #\\. #\\s #\\n #\\space #\\newline ", -1);
    REQUIRE(port, "string_input_port_new");

    scm_object *expected[] = 
        { scm_chars['a'], scm_chars['A'], scm_chars['\\'], scm_chars[' '],
          scm_chars['('], scm_chars[')'], scm_chars['#'], scm_chars['\''],
          scm_chars['`'], scm_chars[','], scm_chars['@'], scm_chars['.'],
          scm_chars['s'], scm_chars['n'], scm_chars[' '], scm_chars['\n'],
        };
    int n = sizeof(expected) / sizeof(scm_object *);

    scm_token *t;
    scm_object *o;
    short type;
    for (i = 0; i < n; ++i) {
        t = scm_token_read(port);
        REQUIRE(t, "scm_token_read");

        type = scm_token_get_type(t);
        o = scm_token_get_obj(t);
        REQUIRE_EQ(type, scm_token_type_char);
        REQUIRE_EQ(o, expected[i]);
        scm_token_free(t);
    }

    scm_object_free(port);
}

TEST(token, identifier) {
    int i;
    init();
    scm_object *port = string_input_port_new("ab1 !+-.@ + - ...", -1);
    REQUIRE(port, "string_input_port_new");

    scm_object *expected[] = {
        scm_chars['a'], scm_chars['b'], scm_chars['1'],
        scm_chars['!'], scm_chars['+'], scm_chars['-'], scm_chars['.'], scm_chars['@'],
        scm_chars['+'],
        scm_chars['-'],
        scm_chars['.'], scm_chars['.'], scm_chars['.'],
    };
    int lens[] = {3, 5, 1, 1, 3};
    int n = 1;

    scm_token *t;
    scm_object *o;
    int c = 0;
    for (i = 0; i < n; ++i) {
        int m = lens[i];

        t = scm_token_read(port);
        REQUIRE(t, "scm_token_read");
        o = scm_token_get_obj(t);

        REQUIRE_EQ(scm_token_get_type(t), scm_token_type_identifier);
        REQUIRE_EQ(scm_string_length(o), m);
        for (int j = 0; j < m; ++j) {
            REQUIRE_EQ(scm_string_ref(o, j), expected[c++]);
        }
        scm_object_free(o);
        scm_token_free(t);
    }

    scm_object_free(port);
}

TEST(token, string) {
    int i;
    init();
    scm_object *port = string_input_port_new("\"a1 (\\\"\\\\\"", -1);
    REQUIRE(port, "string_input_port_new");

    scm_object *expected[] = 
        { scm_chars['a'], scm_chars['1'], scm_chars[' '], scm_chars['('],
          scm_chars['"'], scm_chars['\\'],
        };
    int n = sizeof(expected) / sizeof(scm_object *);

    scm_token *t;
    scm_object *o;
    t = scm_token_read(port);
    REQUIRE(t, "scm_token_read");
    o = scm_token_get_obj(t);

    REQUIRE_EQ(scm_token_get_type(t), scm_token_type_string);
    REQUIRE_EQ(scm_string_length(o), n);
    for (i = 0; i < n; ++i) {
        REQUIRE_EQ(scm_string_ref(o, i), expected[i]);
    }

    scm_object_free(o);
    scm_token_free(t);
    scm_object_free(port);
}

TEST(token, integer) {
    int i;
    init();
    scm_object *port = string_input_port_new("12 +1 -3 #b10 #o10 #d10 #x10 #e5.1 #e1e2", -1);
    REQUIRE(port, "string_input_port_new");

    char *expected[] = 
        { "12", "1", "-3", "2", "8", "10", "16", "5", "100",
        };
    int n = sizeof(expected) / sizeof(char *);

    scm_token *t;
    scm_object *o;
    char *str;
    for (i = 0; i < n; ++i) {
        t = scm_token_read(port);
        REQUIRE(t, "scm_token_read");
        o = scm_token_get_obj(t);

        REQUIRE_EQ(scm_token_get_type(t), scm_token_type_number);
        REQUIRE_EQ(o->type, scm_type_integer);
        str = scm_number_to_string(o, 10);
        REQUIRE_STREQ(str, expected[i]);

        free(str);
        scm_object_free(o);
        scm_token_free(t);
    }

    scm_object_free(port);
}

TEST(token, float) {
    int i;
    init();
    scm_object *port = string_input_port_new("1.2 .5 -.9 1## 0.3# 2e2 +1e-1 1.2E+2 #i5 #i#xa #i#o-10", -1);
    REQUIRE(port, "string_input_port_new");

    char *expected[] = 
        { "1.2", "0.5", "-0.9", "100.0", "0.3", "200.0", "0.1", "120.0", "5.0", "10.0", "-8.0",
        };
    int n = sizeof(expected) / sizeof(char *);

    scm_token *t;
    scm_object *o;
    char *str;
    for (i = 0; i < n; ++i) {
        t = scm_token_read(port);
        REQUIRE(t, "scm_token_read");
        o = scm_token_get_obj(t);

        REQUIRE_EQ(scm_token_get_type(t), scm_token_type_number);
        REQUIRE_EQ(o->type, scm_type_float);
        str = scm_number_to_string(o, 10);
        REQUIRE_STREQ(str, expected[i]);

        free(str);
        scm_object_free(o);
        scm_token_free(t);
    }

    scm_object_free(port);
}

/* bad syntax */
TEST(token, bad_char) {
    int i;
    init();
    char *inputs[] = {
        "#\\", "#\\aa", "#\\spac", "#\\space2", "#\\newlin", "#\\newline2",
    };
    int n = sizeof(inputs) / sizeof(char *);

    for (i = 0; i < n; ++i) {
        scm_object *port = string_input_port_new(inputs[i], -1);
        REQUIRE(port, "string_input_port_new");
        scm_token *t = scm_token_read(port);
        REQUIRE(!t, "scm_token_read");
        scm_object_free(port);
    }
}

TEST(token, bad_identifier) {
    int i;
    init();
    char *inputs[] = {
        /* the first character isn't in <initial> */
        "3a", "\\a", "+a", "-a", ".a", "@a", "..", "....", "我"
    };
    int n = sizeof(inputs) / sizeof(char *);

    for (i = 0; i < n; ++i) {
        scm_object *port = string_input_port_new(inputs[i], -1);
        REQUIRE(port, "string_input_port_new");
        scm_token *t = scm_token_read(port);
        REQUIRE(!t, "scm_token_read");
        scm_object_free(port);
    }
}

TEST(token, bad_string) {
    int i;
    init();
    char *inputs[] = {
        "\"ab", "\"a\\a\"",
    };
    int n = sizeof(inputs) / sizeof(char *);

    for (i = 0; i < n; ++i) {
        scm_object *port = string_input_port_new(inputs[i], -1);
        REQUIRE(port, "string_input_port_new");
        scm_token *t = scm_token_read(port);
        REQUIRE(!t, "scm_token_read");
        scm_object_free(port);
    }
}

TEST(token, bad_number) {
    int i;
    init();
    char *inputs[] = {
        "#10", "#c12", "#e#i1", "#x#d1", "1#o",    /* prefix */
        "#e", "#d.", "#x12g", "1a", "#o9", "#b3", /* no digit, bad digit */
        /* sign must be at the begin, # must be after the digit(s) but no digit(s) after # */
        "++1", "+-1", "1+", "1.2.3", ".#", "-#", "-.#", "1#1", "1#.#0",
        "#x1.2", "#o1#", "#b1e1", "#x1e2",   /* float must be decimal */ 
        "1e++1", "1e+-1", "1e1+", "1e1#", "1e1.2", "1e.1", "1e", "1e+", "1e1a", /* suffix */
        "9223372036854775808", "-9223372036854775809", "1e1000", "1e-1000", /* out of range */
    };
    int n = sizeof(inputs) / sizeof(char *);

    for (i = 0; i < n; ++i) {
        scm_object *port = string_input_port_new(inputs[i], -1);
        REQUIRE(port, "string_input_port_new");
        scm_token *t = scm_token_read(port);
        REQUIRE(!t, "scm_token_read");
        scm_object_free(port);
    }
}


