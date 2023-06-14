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

    char *buf = "a 1";
    scm_object *port = string_input_port_new(buf, 3);
    REQUIRE(port, "string_input_port_new");

    scm_object *a = scm_input_port_peekc(port);
    REQUIRE_EQ(a, scm_chars['a']);
    a = scm_input_port_readc(port);
    REQUIRE_EQ(a, scm_chars['a']);

    scm_object *space = scm_input_port_peekc(port);
    REQUIRE_EQ(space, scm_chars[' ']);
    space = scm_input_port_readc(port);
    REQUIRE_EQ(space, scm_chars[' ']);

    scm_object *one = scm_input_port_peekc(port);
    REQUIRE_EQ(one, scm_chars['1']);
    one = scm_input_port_readc(port);
    REQUIRE_EQ(one, scm_chars['1']);

    scm_object *eof = scm_input_port_peekc(port);
    REQUIRE_EQ(eof, scm_eof);
    eof = scm_input_port_readc(port);
    REQUIRE_EQ(eof, scm_eof);

    eof = scm_input_port_peekc(port);
    REQUIRE_EQ(eof, scm_eof);
    eof = scm_input_port_readc(port);
    REQUIRE_EQ(eof, scm_eof);

    scm_object_free(port);
}
