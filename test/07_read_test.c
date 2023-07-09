#include "test.h"

TAU_MAIN()

TEST(read, simple_datum) {
    scm_object *o = NULL;
    TEST_INIT();

    scm_object *port = string_input_port_new("1 0.1 \"a\" a  #t #f #\\a", -1);
    REQUIRE(port, "string_input_port_new");

    scm_type expected[] = {
        scm_type_integer, scm_type_float, scm_type_string, scm_type_identifier,
        scm_type_true, scm_type_false, scm_type_char, scm_type_eof,
    };

    int n = sizeof(expected) / sizeof(scm_type);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC(o = scm_read(port));
        REQUIRE_EQ(o->type, expected[i]);
        scm_object_free(o);
    }

    scm_object_free(port);
}

TEST(read, list) {
    scm_object *l1, *l2, *l3, *l4, *o;
    TEST_INIT();

    scm_object *port = string_input_port_new("() (1) (1 . 2) ((1))", -1);
    REQUIRE(port, "string_input_port_new");

    REQUIRE_NOEXC(l1 = scm_read(port));
    REQUIRE_EQ(l1, scm_null);

    REQUIRE_NOEXC(l2 = scm_read(port));
    REQUIRE_EQ(l2->type, scm_type_pair);
    o = scm_car(l2);
    REQUIRE_EQ(o->type, scm_type_integer);
    o = scm_cdr(l2);
    REQUIRE_EQ(o, scm_null);

    REQUIRE_NOEXC(l3 = scm_read(port));
    REQUIRE_EQ(l3->type, scm_type_pair);
    o = scm_car(l3);
    REQUIRE_EQ(o->type, scm_type_integer);
    o = scm_cdr(l3);
    REQUIRE_EQ(o->type, scm_type_integer);

    REQUIRE_NOEXC(l4 = scm_read(port));
    REQUIRE_EQ(l4->type, scm_type_pair);
    o = scm_car(l4);
    REQUIRE_EQ(o->type, scm_type_pair);
    o = scm_cdr(l4);
    REQUIRE_EQ(o, scm_null);

    scm_object_free(l1);
    scm_object_free(l2);
    scm_object_free(l3);
    scm_object_free(l4);
    scm_object_free(port);
}

TEST(read, vector) {
    scm_object *v1, *v2, *v3, *o;
    TEST_INIT();

    scm_object *port = string_input_port_new("#() #(1) #(#(1))", -1);
    REQUIRE(port, "string_input_port_new");

    REQUIRE_NOEXC(v1 = scm_read(port));
    REQUIRE_EQ(v1->type, scm_type_vector);
    REQUIRE_EQ(v1, scm_empty_vector);

    REQUIRE_NOEXC(v2 = scm_read(port));
    REQUIRE_EQ(v2->type, scm_type_vector);
    REQUIRE_EQ(scm_vector_length(v2), 1);
    o = scm_vector_ref(v2, 0);
    REQUIRE(o, "scm_vector_ref(v2, 0)");
    REQUIRE_EQ(o->type, scm_type_integer);

    REQUIRE_NOEXC(v3 = scm_read(port));
    REQUIRE_EQ(v3->type, scm_type_vector);
    REQUIRE_EQ(scm_vector_length(v3), 1);
    o = scm_vector_ref(v3, 0);
    REQUIRE(o, "scm_vector_ref(v3, 0)");
    REQUIRE_EQ(o->type, scm_type_vector);

    scm_object_free(v1);
    scm_object_free(v2);
    scm_object_free(v3);
    scm_object_free(port);
}

TEST(read, quote_quasiquote) {
    scm_object *quote, *o;
    TEST_INIT();

    scm_object *port = string_input_port_new("'1 '(1) '#(1) `1 `(1) `#(1)", -1);
    REQUIRE(port, "string_input_port_new");

    char *expected_id[] = {
        "quote", "quote", "quote",
        "quasiquote", "quasiquote", "quasiquote",
    };
    scm_type expected_type[] = {
        scm_type_integer, scm_type_pair, scm_type_vector,
        scm_type_integer, scm_type_pair, scm_type_vector,
    };

    int n = sizeof(expected_type) / sizeof(scm_type);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC(quote = scm_read(port));
        REQUIRE_EQ(quote->type, scm_type_pair);
        REQUIRE_EQ(scm_list_length(quote), 2);
        o = scm_list_ref(quote, 0);
        REQUIRE_EQ(o->type, scm_type_identifier);
        REQUIRE_STREQ(scm_symbol_get_string(o), expected_id[i]);

        o = scm_list_ref(quote, 1);
        REQUIRE_EQ(o->type, expected_type[i]);
        scm_object_free(quote);
    }

    scm_object_free(port);
}

TEST(read, unquote_unquote_splicing) {
    scm_object *unquote, *o;
    TEST_INIT();

    scm_object *port = string_input_port_new(",1 ,(1) ,#(1) ,@1 ,@(1) ,@#(1)", -1);
    REQUIRE(port, "string_input_port_new");

    char *expected_id[] = {
        "unquote", "unquote", "unquote",
        "unquote-splicing", "unquote-splicing", "unquote-splicing",
    };
    scm_type expected_type[] = {
        scm_type_integer, scm_type_pair, scm_type_vector,
        scm_type_integer, scm_type_pair, scm_type_vector,
    };

    int n = sizeof(expected_type) / sizeof(scm_type);
    for (int i = 0; i < n; ++i) {
        REQUIRE_NOEXC(unquote = scm_read(port));
        REQUIRE_EQ(unquote->type, scm_type_pair);
        REQUIRE_EQ(scm_list_length(unquote), 2);
        o = scm_list_ref(unquote, 0);
        REQUIRE_EQ(o->type, scm_type_identifier);
        REQUIRE_STREQ(scm_symbol_get_string(o), expected_id[i]);

        o = scm_list_ref(unquote, 1);
        REQUIRE_EQ(o->type, expected_type[i]);
        scm_object_free(unquote);
    }

    scm_object_free(port);
}

TEST(read, invalid_token) {
    TEST_INIT();

    scm_object *p1 = string_input_port_new(")", -1);
    REQUIRE(p1, "string_input_port_new");
    scm_object *p2 = string_input_port_new(".", -1);
    REQUIRE(p2, "string_input_port_new");
    scm_object *p3 = string_input_port_new("#(1 . 2)", -1);
    REQUIRE(p3, "string_input_port_new");
    scm_object *p4 = string_input_port_new("(. 1)", -1);
    REQUIRE(p4, "string_input_port_new");
    scm_object *p5 = string_input_port_new("(1 . . 2)", -1);
    REQUIRE(p5, "string_input_port_new");
    scm_object *p6 = string_input_port_new("(1 . )", -1);
    REQUIRE(p6, "string_input_port_new");
    scm_object *p7 = string_input_port_new("(1 . 2 3)", -1);
    REQUIRE(p7, "string_input_port_new");
    scm_object *p8 = string_input_port_new("(1 ", -1);
    REQUIRE(p8, "string_input_port_new");
    scm_object *p9 = string_input_port_new("#(1 ", -1);
    REQUIRE(p9, "string_input_port_new");

    scm_object *ports[] = {
        p1, p2, p3, p4, p5, p6, p7, p8, p9,
    };

    char *msgs[] = {
        "parser: unexpected `)`",
        "parser: illegal use of `.`",
        "parser: illegal use of `.`",
        "parser: illegal use of `.`",
        "parser: illegal use of `.`",
        "parser: unexpected `)`",
        "parser: illegal use of `.`",
        "parser: missing right parenthese",
        "parser: missing right parenthese",
    };

    int n = 9;
    for (int i = 0; i < n; ++i) {
        CHECK_EXC(msgs[i], scm_read(ports[i]));
        scm_object_free(ports[i]);
    }
}
