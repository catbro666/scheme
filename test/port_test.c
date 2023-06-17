#include "../src/port.h"
#include <tau/tau.h>

TAU_MAIN()

TEST(port, string_input_port) {
    int res;
    res = scm_object_env_init();
    REQUIRE(!res, "scm_object_env_init");
    res = scm_port_env_init();
    REQUIRE(!res, "scm_port_env_init");
    res = scm_port_env_init();
    REQUIRE(!res, "call scm_port_env_init the second time");

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
