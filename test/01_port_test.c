#include "test.h"

TAU_MAIN()

TEST(port, string_input_port) {
    int res;
    TEST_INIT();

    char *buf = "a 0";
    scm_object *port = string_input_port_new(buf, 3);
    REQUIRE(port, "string_input_port_new");

    int a = scm_input_port_peekc(port);
    REQUIRE_EQ(a, 'a');
    a = scm_input_port_readc(port);
    REQUIRE_EQ(a, 'a');

    int space = scm_input_port_peekc(port);
    REQUIRE_EQ(space, ' ');
    space = scm_input_port_readc(port);
    REQUIRE_EQ(space, ' ');

    res = scm_input_port_unreadc(port, '1');
    REQUIRE(!res, "scm_input_port_unreadc");
    res = scm_input_port_unreadc(port, '2');
    REQUIRE(!res, "scm_input_port_unreadc");
    res = scm_input_port_unreadc(port, '3');
    REQUIRE(!res, "scm_input_port_unreadc");
    res = scm_input_port_unreadc(port, '4');
    REQUIRE(!res, "scm_input_port_unreadc");
    res = scm_input_port_unreadc(port, '5');
    REQUIRE(res, "expected scm_input_port_unreadc to fail when exceeding CACHE_SIZE");

    int four = scm_input_port_readc(port);
    REQUIRE_EQ(four, '4');
    int three = scm_input_port_readc(port);
    REQUIRE_EQ(three, '3');
    int two = scm_input_port_readc(port);
    REQUIRE_EQ(two, '2');
    int one = scm_input_port_readc(port);
    REQUIRE_EQ(one, '1');

    int zero = scm_input_port_peekc(port);
    REQUIRE_EQ(zero, '0');
    zero = scm_input_port_readc(port);
    REQUIRE_EQ(zero, '0');

    int eof = scm_input_port_peekc(port);
    REQUIRE_EQ(eof, -1);
    eof = scm_input_port_readc(port);
    REQUIRE_EQ(eof, -1);

    eof = scm_input_port_peekc(port);
    REQUIRE_EQ(eof, -1);
    eof = scm_input_port_readc(port);
    REQUIRE_EQ(eof, -1);

    res = scm_input_port_unreadc(port, '6');
    REQUIRE(!res, "scm_input_port_unreadc");
    int six = scm_input_port_readc(port);
    REQUIRE_EQ(six, '6');
    eof = scm_input_port_readc(port);
    REQUIRE_EQ(eof, -1);

    scm_object_free(port);
}

TEST(port, string_output_port) {
    TEST_INIT();

    char buf[32];

    scm_object *port = string_output_port_new(buf, 3);
    REQUIRE(port, "string_output_port_new");

    REQUIRE_EQ(scm_output_port_writec(port, 'a'), 1);
    REQUIRE_EQ(scm_output_port_writec(port, 'b'), 1);
    REQUIRE_EQ(scm_output_port_writec(port, '\0'), 1);
    REQUIRE_EQ(scm_output_port_writec(port, 'x'), 0);

    REQUIRE_STREQ(buf, "ab");

    scm_object_free(port);
}

TEST(port, equivalence) {
    TEST_INIT();

    char buf[32];

    scm_object *ports[] = {
        string_input_port_new(buf, 32),
        string_input_port_new(buf, 32),
        string_output_port_new(buf, 32),
        string_output_port_new(buf, 32),
    };

    for (int i = 0; i < 4; ++i) {
        for (int j = i; j < 4; ++j) {
            CHECK_EQ(scm_object_eq(ports[i], ports[j]), i == j, "i=%d,j=%d", i, j);
            CHECK_EQ(scm_object_eqv(ports[i], ports[j]), i == j, "i=%d,j=%d", i, j);
            CHECK_EQ(scm_object_equal(ports[i], ports[j]), i == j, "i=%d,j=%d", i, j);
        }
        scm_object_free(ports[i]);
    }
}
