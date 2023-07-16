#include "object.h"
#include "char.h"
#include "port.h"
#include "string.h"
#include "symbol.h"
#include "number.h"
#include "token.h"
#include "pair.h"
#include "vector.h"
#include "env.h"
#include "exp.h"

#include <stdio.h>

static scm_object *init_global_env() {
    scm_object * env = scm_env_new();
    env = scm_env_extend(env, scm_null, scm_null);

    scm_object_init_env(env);
    scm_port_init_env(env);
    scm_char_init_env(env);
    scm_number_init_env(env);
    scm_string_init_env(env);
    scm_symbol_init_env(env);
    scm_pair_init_env(env);
    scm_vector_init_env(env);
    return env;
}

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
    scm_exp_init();

    scm_object * env = init_global_env();

    printf("Hello world\n");
    return 0;
}

