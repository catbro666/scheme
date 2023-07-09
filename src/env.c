#include "env.h"

#include "symbol.h"
#include "pair.h"

#include <stdlib.h>

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
    return scm_set_cdr(binding, val);
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
    return scm_set_car(env, frame);
}

static int env_is_emtpy(scm_object *env) {
    return env == scm_null;
} 

static scm_object *frame_scan(scm_object *frame, scm_object *var) {
    scm_object *binding = NULL;
    while (!frame_is_empty(frame)) {
        binding = frame_first_binding(frame);
        if (scm_object_eq(binding_get_var(binding), var)) {
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
    scm_object *binding = env_scan(env, var, 1);
    if (binding) {
        return binding_get_val(binding);
    }
    return NULL;    /* throw error or not */
}

int scm_env_set_var(scm_object *env, scm_object *var, scm_object *val) {
    scm_object *binding = env_scan(env, var, 1);
    if (binding) {
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
