#include "object.h"
#include "char.h"
#include "port.h"
#include "string.h"
#include "symbol.h"
#include "number.h"
#include "token.h"
#include "pair.h"
#include "vector.h"

#include <stdio.h>

int main()
{
    scm_object_init();
    scm_char_init();
    scm_port_init();
    scm_string_init();
    scm_symbol_init();
    scm_number_init();
    scm_token_init();
    scm_pair_init();
    scm_vector_init();

    printf("Hello world\n");
    return 0;
}

