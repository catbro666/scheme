#include "../src/foo.h"
#include <tau/tau.h>

#include <stdio.h>

TAU_MAIN()

TEST(foo, foo) {
    foo();
    CHECK(1);
}
