#include "env.h"

#include "port.h"
#include "char.h"
#include "number.h"
#include "string.h"
#include "symbol.h"
#include "pair.h"
#include "vector.h"
#include "eval.h"

#include <stdlib.h>
#include <assert.h>

static scm_object *global_env = NULL;
/* represent binding as pair
 * represent frame as a list of bindings
 * represent env as a list of frames */
static scm_object *binding_new(scm_object *var, scm_object *val) {
    return scm_cons(var, val);
}

static scm_object *binding_get_var(scm_object *binding) {
    return scm_car(binding);
}

static scm_object * binding_get_val(scm_object *binding) {
    return scm_cdr(binding);
}

static void binding_set_val(scm_object *binding, scm_object *val) {
    scm_set_cdr(binding, val);
}

static scm_object *frame_add_binding(scm_object *frame, scm_object *var, scm_object *val) {
    return scm_cons(binding_new(var, val), frame);
}

static scm_object *frame_first_binding(scm_object *frame) {
    return scm_car(frame);
}

static scm_object *frame_rest_bindings(scm_object *frame) {
    return scm_cdr(frame);
}

static int frame_is_empty(scm_object *frame) {
    return frame == scm_null;
}

/* vars may be improper list, vals must be list */
static scm_object *frame_new(scm_object *vars, scm_object *vals) {
    scm_object *frame = scm_null; 
    while (vars->type == scm_type_pair) {
        if (vals == scm_null) {
            goto err;   /* too few arguments */
        }
        frame = frame_add_binding(frame, scm_car(vars), scm_car(vals));
        vars = scm_cdr(vars);
        vals = scm_cdr(vals);
    }

    if (vars == scm_null) {
        if (vals != scm_null) {
            goto err;   /* too many arguments */
        }
    }
    else {  /* improper list */
        frame = frame_add_binding(frame, vars, vals);
    }

    return frame;
err:
    scm_object_free(frame);
    return NULL;
}

static scm_object *env_first_frame(scm_object *env) {
    return scm_car(env);
}

static scm_object *env_rest_frames(scm_object *env) {
    return scm_cdr(env);
}

static void env_set_first_frame(scm_object *env, scm_object* frame) {
    scm_set_car(env, frame);
}

static int env_is_emtpy(scm_object *env) {
    return env == scm_null;
} 

static scm_object *frame_scan(scm_object *frame, scm_object *var) {
    scm_object *binding = NULL;
    while (!frame_is_empty(frame)) {
        binding = frame_first_binding(frame);
        if (scm_eq(binding_get_var(binding), var)) {
            return binding;
        }
        frame = frame_rest_bindings(frame);
    }

    return NULL;
}

static scm_object *env_scan(scm_object *env, scm_object *var, int all_frames) {
    scm_object *frame = NULL;
    scm_object *binding = NULL;
    do {
        frame = env_first_frame(env);
        binding = frame_scan(frame, var);
        if (binding) {
            return binding;
        }
        env = env_rest_frames(env);
    } while(all_frames && !env_is_emtpy(env));

    return NULL;
}

scm_object *scm_env_new() {
    return scm_null;
}

scm_object *scm_env_extend(scm_object *env, scm_object *vars, scm_object *vals) {
    scm_object *frame = frame_new(vars, vals);
    if (frame) {
        return scm_cons(frame, env);
    }
    else {
        return NULL;
    }
}

scm_object *scm_env_lookup_var(scm_object *env, scm_object *var) {
    if (IS_EXTENDED_IDENTIFIER(var)) {
        env = scm_esymbol_get_env(var);
        assert(IS_RAW_IDENTIFIER(scm_esymbol_get_symbol(var)));
    }
    scm_object *binding = env_scan(env, var, 1);
    if (binding) {
        return binding_get_val(binding);
    }
    return NULL;    /* throw error or not */
}

int scm_env_set_var(scm_object *env, scm_object *var, scm_object *val) {
    scm_object *binding = env_scan(env, var, 1);
    if (binding) {
        scm_object *oval = binding_get_val(binding);
        if (oval->type == scm_type_core_syntax ||
            oval->type == scm_type_transformer)
            return 2;   /* cannot mutate syntax identifier */
        binding_set_val(binding, val);
        return 0;
    }
    else {
        return 1;   /* cannot set variable before its definition */
    }
}

void scm_env_define_var(scm_object *env, scm_object *var, scm_object *val) {
    scm_object *binding = env_scan(env, var, 0);
    if (binding) {
        binding_set_val(binding, val);
    }
    else {
        env_set_first_frame(env, frame_add_binding(env_first_frame(env), var, val));
    }
}

int scm_env_add_prim(scm_object *env, const char *name, prim_fn fn,
                     int min_arity, int max_arity, scm_object *preds) {
    scm_object *prim = scm_primitive_new(name, fn, min_arity, max_arity, preds);
    scm_env_define_var(env, scm_symbol_new(name, -1), prim);
    return 0;
}

scm_object *scm_global_env() {
    if (global_env)
        return global_env;

    global_env = scm_env_new();
    global_env = scm_env_extend(global_env, scm_null, scm_null);

    scm_object_init_env(global_env);
    scm_port_init_env(global_env);
    scm_char_init_env(global_env);
    scm_number_init_env(global_env);
    scm_string_init_env(global_env);
    scm_symbol_init_env(global_env);
    scm_pair_init_env(global_env);
    scm_vector_init_env(global_env);
    scm_eval_init_env(global_env);
    return global_env;
}


