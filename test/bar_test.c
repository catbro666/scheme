#include "../src/bar.h"
#include <tau/tau.h>
#include <stdio.h>

TAU_MAIN()

TEST(bar, bar) {
    bar();
    CHECK(1);
}
