#include "../src/symbol.h"
#include <tau/tau.h>

TAU_MAIN()

void init() {
    int res;
    res = scm_symbol_env_init();
    REQUIRE(!res, "scm_symbol_env_init");
}

TEST(symbol, new) {
    init();

    scm_object *obj = scm_symbol_new("ab1", 3);
    REQUIRE(obj, "scm_symbol_new");

    REQUIRE_EQ(obj->type, scm_type_symbol);

    char *res = scm_symbol_get_string(obj);

    REQUIRE_STREQ(res, "ab1");

    scm_object_free(obj);
}

