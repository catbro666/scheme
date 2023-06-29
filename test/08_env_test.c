#include "../src/object.h"
#include "../src/char.h"
#include "../src/number.h"
#include "../src/string.h"
#include "../src/symbol.h"
#include "../src/port.h"
#include "../src/token.h"
#include "../src/pair.h"
#include "../src/vector.h"
#include "../src/read.h"
#include "../src/env.h"
#include <tau/tau.h>

TAU_MAIN()

void init() {
    int res;
    res = scm_object_env_init();
    REQUIRE(!res, "scm_object_env_init");
    res = scm_char_env_init();
    REQUIRE(!res, "scm_char_env_init");
    res = scm_port_env_init();
    REQUIRE(!res, "scm_port_env_init");
    res = scm_string_env_init();
    REQUIRE(!res, "scm_string_env_init");
    res = scm_symbol_env_init();
    REQUIRE(!res, "scm_symbol_env_init");
    res = scm_number_env_init();
    REQUIRE(!res, "scm_number_env_init");
    res = scm_token_env_init();
    REQUIRE(!res, "scm_token_env_init");
    res = scm_pair_env_init();
    REQUIRE(!res, "scm_pair_env_init");
    res = scm_vector_env_init();
    REQUIRE(!res, "scm_vector_env_init");
}

TEST(env, normal) {
    scm_object *o = NULL;
    int ret;
    init();

    scm_object *env = scm_env_new();
    REQUIRE_EQ(env, scm_null);

    scm_object *a = scm_symbol_new("a", 1);
    scm_object *b = scm_symbol_new("b", 1);
    scm_object *c = scm_symbol_new("c", 1);
    scm_object *d = scm_symbol_new("d", 1);

    /* extend then lookup */
    env = scm_env_extend(env, scm_list(2, a, b),
                         scm_list(2, scm_chars['a'], scm_chars['b']));
    REQUIRE(env, "scm_env_extend");

    o = scm_env_lookup_var(env, a);
    REQUIRE(o, "scm_env_lookup_var(env, a)");
    REQUIRE_EQ(o, scm_chars['a']);  
    o = scm_env_lookup_var(env, b);
    REQUIRE(o, "scm_env_lookup_var(env, b)");
    REQUIRE_EQ(o, scm_chars['b']); 
    o = scm_env_lookup_var(env, c);
    REQUIRE(!o, "scm_env_lookup_var(env, c)");
 
    /* set then lookup */  
    ret = scm_env_set_var(env, a, scm_true);
    REQUIRE(!ret, "scm_env_set_var(env, a, scm_true)");
    ret = scm_env_set_var(env, b, scm_false);
    REQUIRE(!ret, "scm_env_set_var(env, b, scm_false)");
    ret = scm_env_set_var(env, c, scm_null);
    REQUIRE(ret, "can't set variable before its definition");

    o = scm_env_lookup_var(env, a);
    REQUIRE(o, "scm_env_lookup_var(env, a)");
    REQUIRE_EQ(o, scm_true);
    o = scm_env_lookup_var(env, b);
    REQUIRE(o, "scm_env_lookup_var(env, b)");
    REQUIRE_EQ(o, scm_false);
    o = scm_env_lookup_var(env, c);
    REQUIRE(!o, "scm_env_lookup_var(env, c)");

    /* define then lookup */
    scm_env_define_var(env, a, scm_chars['a']);
    scm_env_define_var(env, b, scm_chars['b']);
    scm_env_define_var(env, c, scm_null);
      
    o = scm_env_lookup_var(env, a);
    REQUIRE(o, "scm_env_lookup_var(env, a)");
    REQUIRE_EQ(o, scm_chars['a']);  
    o = scm_env_lookup_var(env, b);
    REQUIRE(o, "scm_env_lookup_var(env, b)");
    REQUIRE_EQ(o, scm_chars['b']);
    o = scm_env_lookup_var(env, c);
    REQUIRE(o, "scm_env_lookup_var(env, c)");
    REQUIRE_EQ(o, scm_null);

    /* extend another frame */
    scm_object *env2 = scm_env_extend(env, scm_list(2, c, d),
                                      scm_list(2, scm_chars['c'], scm_chars['d']));
    REQUIRE(env2, "scm_env_extend");

    o = scm_env_lookup_var(env2, a);
    REQUIRE(o, "scm_env_lookup_var(env2, a)");
    REQUIRE_EQ(o, scm_chars['a']);
    o = scm_env_lookup_var(env2, b);
    REQUIRE(o, "scm_env_lookup_var(env2, b)");
    REQUIRE_EQ(o, scm_chars['b']);
    o = scm_env_lookup_var(env2, c);
    REQUIRE(o, "scm_env_lookup_var(env2, c)");
    REQUIRE_EQ(o, scm_chars['c']);
    o = scm_env_lookup_var(env2, d);
    REQUIRE(o, "scm_env_lookup_var(env2, d)");
    REQUIRE_EQ(o, scm_chars['d']);

    /* env remains unchanged */
    o = scm_env_lookup_var(env, a);
    REQUIRE(o, "scm_env_lookup_var(env, a)");
    REQUIRE_EQ(o, scm_chars['a']);  
    o = scm_env_lookup_var(env, b);
    REQUIRE(o, "scm_env_lookup_var(env, b)");
    REQUIRE_EQ(o, scm_chars['b']);
    o = scm_env_lookup_var(env, c);
    REQUIRE(o, "scm_env_lookup_var(env, c)");
    REQUIRE_EQ(o, scm_null);
    o = scm_env_lookup_var(env, d);
    REQUIRE(!o, "scm_env_lookup_var(env, d)");

    /* set then lookup */  
    ret = scm_env_set_var(env2, a, scm_chars['1']);
    REQUIRE(!ret, "scm_env_set_var(env2, a, scm_chars['1'])");
    ret = scm_env_set_var(env2, b, scm_chars['2']);
    REQUIRE(!ret, "scm_env_set_var(env2, b, scm_chars['2'])");
    ret = scm_env_set_var(env2, c, scm_chars['3']);
    REQUIRE(!ret, "scm_env_set_var(env2, c, scm_chars['3'])");
    ret = scm_env_set_var(env2, d, scm_chars['4']);
    REQUIRE(!ret, "scm_env_set_var(env2, d, scm_chars['4'])");

    o = scm_env_lookup_var(env2, a);
    REQUIRE(o, "scm_env_lookup_var(env2, a)");
    REQUIRE_EQ(o, scm_chars['1']);
    o = scm_env_lookup_var(env2, b);
    REQUIRE(o, "scm_env_lookup_var(env2, b)");
    REQUIRE_EQ(o, scm_chars['2']);
    o = scm_env_lookup_var(env2, c);
    REQUIRE(o, "scm_env_lookup_var(env2, c)");
    REQUIRE_EQ(o, scm_chars['3']);
    o = scm_env_lookup_var(env2, d);
    REQUIRE(o, "scm_env_lookup_var(env2, d)");
    REQUIRE_EQ(o, scm_chars['4']);

    /* a, b changed in env */
    o = scm_env_lookup_var(env, a);
    REQUIRE(o, "scm_env_lookup_var(env, a)");
    REQUIRE_EQ(o, scm_chars['1']);  
    o = scm_env_lookup_var(env, b);
    REQUIRE(o, "scm_env_lookup_var(env, b)");
    REQUIRE_EQ(o, scm_chars['2']);
    o = scm_env_lookup_var(env, c);
    REQUIRE(o, "scm_env_lookup_var(env, c)");
    REQUIRE_EQ(o, scm_null);
    o = scm_env_lookup_var(env, d);
    REQUIRE(!o, "scm_env_lookup_var(env, d)");

    /* define then lookup */
    scm_env_define_var(env2, a, scm_chars['a']);
    scm_env_define_var(env2, b, scm_chars['b']);
    scm_env_define_var(env2, c, scm_chars['c']);
    scm_env_define_var(env2, d, scm_chars['d']);

    o = scm_env_lookup_var(env2, a);
    REQUIRE(o, "scm_env_lookup_var(env2, a)");
    REQUIRE_EQ(o, scm_chars['a']);
    o = scm_env_lookup_var(env2, b);
    REQUIRE(o, "scm_env_lookup_var(env2, b)");
    REQUIRE_EQ(o, scm_chars['b']);
    o = scm_env_lookup_var(env2, c);
    REQUIRE(o, "scm_env_lookup_var(env2, c)");
    REQUIRE_EQ(o, scm_chars['c']);
    o = scm_env_lookup_var(env2, d);
    REQUIRE(o, "scm_env_lookup_var(env2, d)");
    REQUIRE_EQ(o, scm_chars['d']);

    /* env remains unchanged */
    o = scm_env_lookup_var(env, a);
    REQUIRE(o, "scm_env_lookup_var(env, a)");
    REQUIRE_EQ(o, scm_chars['1']);  
    o = scm_env_lookup_var(env, b);
    REQUIRE(o, "scm_env_lookup_var(env, b)");
    REQUIRE_EQ(o, scm_chars['2']);
    o = scm_env_lookup_var(env, c);
    REQUIRE(o, "scm_env_lookup_var(env, c)");
    REQUIRE_EQ(o, scm_null);
    o = scm_env_lookup_var(env, d);
    REQUIRE(!o, "scm_env_lookup_var(env, d)");

    scm_object_free(env2);
 }

/* arguments mismatch, improper list */
TEST(env, arguments_mismatch_and_improper_list) {
    scm_object *o = NULL;
    init();

    scm_object *env = scm_env_new();
    REQUIRE_EQ(env, scm_null);

    scm_object *a = scm_symbol_new("a", 1);
    scm_object *b = scm_symbol_new("b", 1);
    scm_object *c = scm_symbol_new("c", 1);

    env = scm_env_extend(env, scm_list(3, a, b, c),
                         scm_list(2, scm_chars['a'], scm_chars['b']));
    REQUIRE(!env, "scm_env_extend, too few arguments");
    env = scm_env_extend(env, scm_list(2, a, b, c),
                         scm_list(3, scm_chars['a'], scm_chars['b'], scm_chars['c']));
    REQUIRE(!env, "scm_env_extend, too many arguments");

    /* improper list */
    env = scm_env_extend(env, scm_cons(a, b), scm_null);
    REQUIRE(!env, "scm_env_extend, too few arguments");

    env = scm_env_extend(env, scm_cons(a, b), scm_list(1, scm_chars['a']));
    REQUIRE(env, "scm_env_extend");
    o = scm_env_lookup_var(env, a);
    REQUIRE(o, "scm_env_lookup_var(env, a)");
    REQUIRE_EQ(o, scm_chars['a']);  
    o = scm_env_lookup_var(env, b);
    REQUIRE(o, "scm_env_lookup_var(env, b)");
    REQUIRE_EQ(o, scm_null);

    env = scm_env_extend(env, scm_cons(a, b), scm_list(2, scm_chars['a'], scm_chars['b']));
    REQUIRE(env, "scm_env_extend");
    o = scm_env_lookup_var(env, a);
    REQUIRE(o, "scm_env_lookup_var(env, a)");
    REQUIRE_EQ(o, scm_chars['a']);  
    o = scm_env_lookup_var(env, b);
    REQUIRE(o, "scm_env_lookup_var(env, b)");
    REQUIRE_EQ(o->type, scm_type_pair);
    REQUIRE_EQ(scm_car(o), scm_chars['b']);
    REQUIRE_EQ(scm_cdr(o), scm_null);
}

