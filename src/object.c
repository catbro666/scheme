#include "object.h"
#include "symbol.h"

#include "port.h"
#include "proc.h"
#include "env.h"

#include <stdlib.h>

static scm_object_methods *all_methods[scm_type_max] = {0};

scm_object *scm_boolean(int b) {
    return b ? scm_true : scm_false;
}

scm_object *scm_and(scm_object *b1, scm_object *b2) {
    return scm_boolean(b1 != scm_false && b2 != scm_false);
}

scm_object *scm_or(scm_object *b1, scm_object *b2) {
    return scm_boolean(b1 != scm_false || b2 != scm_false);
}

scm_object *scm_not(scm_object *b1) {
    return scm_boolean(b1 == scm_false);
}

void scm_object_free(scm_object *obj) {
    scm_object_methods *methods = all_methods[obj->type];
    if (methods && methods->free)
        methods->free(obj);
}

int scm_eqv(scm_object *o1, scm_object *o2) {
    if (o1->type != o2->type)
        return 0;
    return all_methods[o1->type]->eqv(o1, o2);
}

int scm_eq(scm_object *o1, scm_object *o2) {
    return scm_eqv(o1, o2);
}

int scm_equal(scm_object *o1, scm_object *o2) {
    if (o1->type != o2->type)
        return 0;
    return all_methods[o1->type]->equal(o1, o2);
}

#define define_prim_eq(name) \
static scm_object *prim_##name(int n, scm_object *args) { \
    (void)n; \
    if (scm_##name(scm_car(args), scm_cadr(args))) \
        return scm_true; \
    else \
        return scm_false; \
}

define_prim_eq(eq);
define_prim_eq(eqv);
define_prim_eq(equal);

/* predicates */
#define define_predicate(name, exp) \
    scm_object *scm_is_##name(scm_object *obj) { \
        return scm_boolean(exp); \
    } \
    define_primitive_1(is_##name); \
    scm_object *pred_##name = NULL


define_predicate(boolean, obj == scm_true || obj == scm_false);
define_predicate(null, obj == scm_null);
define_predicate(char, obj->type == scm_type_char);
define_predicate(integer, obj->type == scm_type_integer);
define_predicate(number, obj->type == scm_type_integer || obj->type == scm_type_float);
define_predicate(string, obj->type == scm_type_string);
define_predicate(symbol, obj->type == scm_type_identifier);
define_predicate(pair, obj->type == scm_type_pair);
define_predicate(vector, obj->type == scm_type_vector);
define_predicate(input_port, obj->type == scm_type_input_port);
define_predicate(output_port, obj->type == scm_type_output_port);
define_predicate(port, obj->type == scm_type_input_port || obj->type == scm_type_output_port);
define_predicate(procedure, obj->type == scm_type_primitive || obj->type == scm_type_compound);
scm_object *pred_real = NULL;


/* singleton objects */

/* can't be free'ed, i.e. can't be sweeped by gc */
static scm_object scm_eof_arr[1] = {{scm_type_eof}};
static scm_object scm_true_arr[1] = {{scm_type_true}};
static scm_object scm_false_arr[1] = {{scm_type_false}};
static scm_object scm_dot_arr[1] = {{scm_type_dot}};
static scm_object scm_rparen_arr[1] = {{scm_type_rparen}};
static scm_object scm_void_arr[1] = {{scm_type_void}};
static scm_object scm_null_arr[1] = {{scm_type_null}};
scm_object *scm_eof = scm_eof_arr;
scm_object *scm_true = scm_true_arr;
scm_object *scm_false = scm_false_arr;
scm_object *scm_dot = scm_dot_arr;
scm_object *scm_rparen = scm_rparen_arr;
scm_object *scm_void = scm_void_arr;
scm_object *scm_null = scm_null_arr;

static int always_eqv(scm_object *o1, scm_object *o2) {
    (void)o1; (void)o2;
    return 1;
}

scm_object_methods simple_methods = { NULL, always_eqv, always_eqv};

static int initialized = 0;

int scm_object_init(void) {
    if (initialized)
        return 0;

    scm_object_register(scm_type_eof, &simple_methods);
    scm_object_register(scm_type_true, &simple_methods);
    scm_object_register(scm_type_false, &simple_methods);
    scm_object_register(scm_type_void, &simple_methods);
    scm_object_register(scm_type_null, &simple_methods);

    pred_null = scm_primitive_new("null?", prim_is_null, 1, 1, NULL);
    pred_boolean = scm_primitive_new("boolean?", prim_is_boolean, 1, 1, NULL);
    pred_char = scm_primitive_new("char?", prim_is_char, 1, 1, NULL);
    pred_integer = scm_primitive_new("integer?", prim_is_integer, 1, 1, NULL);
    pred_real = scm_primitive_new("real?", prim_is_number, 1, 1, NULL);
    pred_number = scm_primitive_new("number?", prim_is_number, 1, 1, NULL);
    pred_string = scm_primitive_new("string?", prim_is_string, 1, 1, NULL);
    pred_symbol = scm_primitive_new("symbol?", prim_is_symbol, 1, 1, NULL);
    pred_pair = scm_primitive_new("pair?", prim_is_pair, 1, 1, NULL);
    pred_vector = scm_primitive_new("vector?", prim_is_vector, 1, 1, NULL);
    pred_input_port = scm_primitive_new("input-port?", prim_is_input_port, 1, 1, NULL);
    pred_output_port = scm_primitive_new("output-port?", prim_is_output_port, 1, 1, NULL);
    pred_port = scm_primitive_new("port?", prim_is_port, 1, 1, NULL);
    pred_procedure = scm_primitive_new("procedure?", prim_is_procedure, 1, 1, NULL);

    initialized = 1;
    return 0;
}

int scm_object_init_env(scm_object *env) {
    scm_env_define_var(env, scm_symbol_new("null?", -1), pred_null);
    scm_env_define_var(env, scm_symbol_new("boolean?", -1), pred_boolean);
    scm_env_define_var(env, scm_symbol_new("char?", -1), pred_char);
    scm_env_define_var(env, scm_symbol_new("integer?", -1), pred_integer);
    scm_env_define_var(env, scm_symbol_new("real?", -1), pred_number);
    scm_env_define_var(env, scm_symbol_new("number?", -1), pred_number);
    scm_env_define_var(env, scm_symbol_new("string?", -1), pred_string);
    scm_env_define_var(env, scm_symbol_new("symbol?", -1), pred_symbol);
    scm_env_define_var(env, scm_symbol_new("pair?", -1), pred_pair);
    scm_env_define_var(env, scm_symbol_new("vector?", -1), pred_vector);
    scm_env_define_var(env, scm_symbol_new("input-port?", -1), pred_input_port);
    scm_env_define_var(env, scm_symbol_new("output-port?", -1), pred_output_port);
    scm_env_define_var(env, scm_symbol_new("port?", -1), pred_port);
    scm_env_define_var(env, scm_symbol_new("procedure?", -1), pred_procedure);

    scm_env_add_prim(env, "eq?", prim_eq, 2, 2, NULL);
    scm_env_add_prim(env, "eqv?", prim_eqv, 2, 2, NULL);
    scm_env_add_prim(env, "equal?", prim_equal, 2, 2, NULL);

    return 0;
}

void scm_object_register(scm_type type, scm_object_methods *methods) {
    all_methods[type] = methods;
}
