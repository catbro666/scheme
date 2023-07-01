#include "../src/char.h"
#include "../src/number.h"
#include "../src/pair.h"
#include <tau/tau.h>

TAU_MAIN()

void init() {
    int res;
    res = scm_char_env_init();
    REQUIRE(!res, "scm_char_env_init");
    res = scm_number_env_init();
    REQUIRE(!res, "scm_number_env_init");
    res = scm_pair_env_init();
    REQUIRE(!res, "scm_pair_env_init");
}

TEST(pair, cons_car_cdr) {
    char *str;
    init();

    scm_object *pair = scm_cons(scm_chars['a'], scm_number_new_integer("1", 10));
    REQUIRE(pair, "scm_cons");
    REQUIRE_EQ(pair->type, scm_type_pair);

    scm_object *car = scm_car(pair);
    REQUIRE(car, "scm_car");
    REQUIRE_EQ(car->type, scm_type_char);
    REQUIRE_EQ(scm_char_get_char(car), 'a');

    scm_object *cdr = scm_cdr(pair);
    REQUIRE(cdr, "scm_cdr");
    REQUIRE_EQ(cdr->type, scm_type_integer);
    str = scm_number_to_string(cdr, 10);
    REQUIRE_STREQ(str, "1");

    free(str);
    scm_object_free(pair);
}

TEST(pair, set_car_cdr) {
    char *str;
    init();

    scm_object *pair = scm_cons(scm_chars['a'], scm_number_new_integer("1", 10));
    REQUIRE(pair, "scm_cons");
    REQUIRE_EQ(pair->type, scm_type_pair);

    scm_set_car(pair, scm_chars['b']);
    scm_object *car = scm_car(pair);
    REQUIRE(car, "scm_car");
    REQUIRE_EQ(car->type, scm_type_char);
    REQUIRE_EQ(scm_char_get_char(car), 'b');

    scm_set_cdr(pair, scm_number_new_integer("2", 10));
    scm_object *cdr = scm_cdr(pair);
    REQUIRE(cdr, "scm_cdr");
    REQUIRE_EQ(cdr->type, scm_type_integer);
    str = scm_number_to_string(cdr, 10);
    REQUIRE_STREQ(str, "2");

    free(str);
    scm_object_free(pair);
}

TEST(pair, list) {
    init();

    scm_object *list = scm_list(2, scm_chars['a'], scm_chars['b']);
    REQUIRE(list, "scm_list");
    REQUIRE_EQ(list->type, scm_type_pair);
    REQUIRE(scm_is_list(list));
    REQUIRE_EQ(scm_list_length(list), 2);

    scm_object *car = scm_car(list);
    REQUIRE(car, "first element of list");
    REQUIRE_EQ(car->type, scm_type_char);
    REQUIRE_EQ(scm_char_get_char(car), 'a');

    scm_object *cdr = scm_cdr(list);
    REQUIRE(cdr, "scm_cdr");
    REQUIRE_EQ(cdr->type, scm_type_pair);
    REQUIRE(scm_is_list(cdr));
    REQUIRE_EQ(scm_list_length(cdr), 1);

    scm_object *cadr = scm_car(cdr);
    REQUIRE(cadr, "second element of list");
    REQUIRE_EQ(car->type, scm_type_char);
    REQUIRE_EQ(scm_char_get_char(cadr), 'b');

    scm_object *cddr = scm_cdr(cdr);
    REQUIRE(cddr, "empty list");
    REQUIRE_EQ(cddr->type, scm_type_null);
    REQUIRE(scm_is_list(cddr));
    REQUIRE_EQ(scm_list_length(cddr), 0);

    scm_object_free(list);

    scm_object *o1 = scm_cons(scm_true, scm_false);
    scm_object *o2 = scm_true;
    REQUIRE(!scm_is_list(o1));
    REQUIRE(!scm_is_list(o2));
    REQUIRE_EQ(scm_list_length(o1), -1);
    REQUIRE_EQ(scm_list_length(o2), -1);
    scm_object_free(o1);
    scm_object_free(o2);
}

TEST(pair, list_ref) {
    scm_object *e;
    init();

    scm_object *list = scm_list(2, scm_chars['a'], scm_chars['b']);
    REQUIRE(list, "scm_list");
    REQUIRE_EQ(list->type, scm_type_pair);

    e = scm_list_ref(list, 0);
    REQUIRE_EQ(e->type, scm_type_char);
    REQUIRE_EQ(scm_char_get_char(e), 'a');

    e = scm_list_ref(list, 1);
    REQUIRE_EQ(e->type, scm_type_char);
    REQUIRE_EQ(scm_char_get_char(e), 'b');

    e = scm_list_ref(list, -1);
    REQUIRE(!e, "invalid index");
    e = scm_list_ref(list, 2);
    REQUIRE(!e, "invalid index");

    scm_object_free(list);
}
