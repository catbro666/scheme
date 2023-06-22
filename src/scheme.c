#include "object.h"
#include "char.h"
#include "port.h"
#include "string.h"
#include "number.h"
#include "token.h"
#include "pair.h"
#include "vector.h"
#include <stdio.h>

int main()
{
    scm_object_env_init();
    scm_char_env_init();
    scm_port_env_init();
    scm_string_env_init();
    scm_number_env_init();
    scm_token_env_init();
    scm_pair_env_init();
    scm_vector_env_init();

    printf("Hello world\n");
    return 0;
}

