#include "xform.h"

#include "err.h"
#include "number.h"
#include "symbol.h"
#include "pair.h"
#include "vector.h"
#include "exp.h"
#include "env.h"
#include "eval.h"

#include <stdlib.h>
#include <assert.h>

/* How to maintain lexical scoping?
 * pattern variable: evaluate the input form in the new env and use the result
 * as the value of the variable.
 * literal variable: evalute it in the new and old env respectively,
 * and compare the values. 
 * local identifier in template: if it's bound in the old env, not rename it;
 * otherwise, rename it. As they may duplicate in recursive macro.
 * It seems not feasible. We can't evalute it in advance as we don't know
 * if it needs to be evaluted until the expanded code is evaluted.
 *
 * Solution 1: extend the identifier to support binding an optional env,
 * and prioritize the evaluation in the given env. We can optionaly represent
 * all the identifiers in input forms as the extended structures or represent all
 * the local identifiers in template as the extended structures.
 **/

typedef struct scm_trasformer_st {
    scm_object base;
    scm_object *kw;
    scm_object *literals;
    scm_object *rules;
    scm_object *env;
} scm_transformer;

static void xformer_free(scm_object *obj) {
    free(obj);
}

#define rule_pattern scm_car
#define rule_template scm_cadr

static int check_subpattern(scm_object *pat, scm_object *literals, scm_object **pids, int depth);
static int check_template(scm_object *temp, scm_object *pids, long depth, long delta, int escape);
static scm_object *match_list_pattern(scm_object *lf, scm_object *lp,
    scm_object *literals, scm_object *oenv, scm_object *nenv);
static scm_object *match_vector_pattern(scm_object *vf, scm_object *vp,
    scm_object *literals, scm_object *oenv, scm_object *nenv);
static scm_object *match_subpattern(scm_object *f, scm_object *p,
    scm_object *literals, scm_object *oenv, scm_object *nenv, int before_ellipsis);
static scm_object *expand_list_template(scm_object *temp, scm_object *pids,
                                        scm_object *env, int uid, int escape);
static scm_object *expand_vector_template(scm_object *temp, scm_object *pids,
                                          scm_object *env, int uid, int escape);

static void syntax_rules_error(scm_object *exp, const char *err) {
    scm_error_object(exp, "syntax-rules: %s in: ", err);
}

/*=============== create transformer ================*/
/* old env is used to tell if the local variables in template need to be renamed.
 * if it's bound, not rename; otherwise, rename */
scm_object *scm_transformer_new(scm_object *kw, scm_object *literals, scm_object *rules, scm_object *env) {
    scm_transformer *xformer = malloc(sizeof(scm_transformer));
    xformer->base.type = scm_type_transformer;
    xformer->kw = kw;
    xformer->literals = literals;
    xformer->rules = rules;
    xformer->env = env;

    return (scm_object *)xformer;
}

/*=============== check syntax rules ================*/
static int is_pattern_id(scm_object *p, scm_object *literals) {
    return IS_IDENTIFIER(p) && !scm_memq(p, literals);
}

static int is_literal_id(scm_object *p, scm_object *literals) {
    return IS_IDENTIFIER(p) && scm_memq(p, literals);
}

static int check_list_pattern(scm_object *pat, scm_object *literals, scm_object **pids, long depth) {
    scm_object *l = pat;
    scm_object *o, *nexto;
    int list_contains_pid = 0;
    int elem_contains_pid = 0;
    int before_ellipsis = 0;
    if (l->type == scm_type_pair) {
        o = scm_car(l);
        if (scm_eq(o, sym_ellipsis))
            syntax_rules_error(pat, "the `...` must be after subpattern including pattern identifiers");
    }
    while (l->type == scm_type_pair) {
        l = scm_cdr(l);
        if (l->type == scm_type_pair) {
            nexto = scm_car(l);
            if (scm_eq(nexto, sym_ellipsis)) {
                if (scm_cdr(l) != scm_null)
                    syntax_rules_error(pat, "the `...` must be at the end of a <pattern> with a proper list or vector form");
                before_ellipsis = 1;
            }
        }

        elem_contains_pid = check_subpattern(o, literals, pids, depth + before_ellipsis);
        list_contains_pid = list_contains_pid || elem_contains_pid;
        if (before_ellipsis) {
            if (!elem_contains_pid)
                syntax_rules_error(pat, "the `...` must be after subpattern including pattern identifiers");
            return list_contains_pid;
        }

        o = nexto;
    }
    if (l != scm_null) {
        if (scm_eq(l, sym_ellipsis))
            syntax_rules_error(l, "the `...` must at the end of a <pattern> with a proper list or vector form");
        elem_contains_pid = check_subpattern(l, literals, pids, depth);
        list_contains_pid = list_contains_pid || elem_contains_pid;
    }

    return list_contains_pid;
}

static int check_vector_pattern(scm_object *pat, scm_object *literals, scm_object **pids, long depth) {
    long len = scm_vector_length(pat);
    int vect_contains_pid = 0;
    int elem_contains_pid = 0;
    int before_ellipsis = 0;
    if (len > 0 && scm_eq(scm_vector_ref(pat, 0), sym_ellipsis))
        syntax_rules_error(pat, "the `...` must be after subpattern including pattern identifiers");

    FOREACH_VECTOR(i, len, pat) {
        if (i < (len - 1)) {
            if (scm_eq(scm_vector_ref(pat, i+1), sym_ellipsis)) {
                if (i != (len - 2))
                    syntax_rules_error(pat, "the `...` must at the end of a <pattern> with a proper list or vector form");
                before_ellipsis = 1;
            }
        }

        elem_contains_pid = check_subpattern(scm_vector_ref(pat, i), literals, pids, depth + before_ellipsis);
        vect_contains_pid = vect_contains_pid || elem_contains_pid;

        if (before_ellipsis) {
            if (!elem_contains_pid)
                syntax_rules_error(pat, "the `...` must be after subpattern including pattern identifiers");
            return vect_contains_pid;
        }
    }
    return vect_contains_pid;
}

static int pattern_var_exists(scm_object *pids, scm_object *pid) {
    FOREACH_LIST(o, pids) {
        if (scm_eq(scm_car(o), pid))
            return 1;
    }
    return 0;
}

static scm_object *add_pattern_var(scm_object *pids, scm_object *pid, long depth) {
    return scm_cons(scm_cons(pid, INTEGER(depth)), pids);
}

static long get_pattern_var_depth(scm_object *pids, scm_object *pid) {
    FOREACH_LIST(o, pids) {
        if (scm_eq(scm_car(o), pid))
            return scm_integer_get_val(scm_cdr(o));
    }
    return -1;
}

/* returns a value denotes if the subpattern contains pid or not */
static int check_subpattern(scm_object *pat, scm_object *literals, scm_object **pids, int depth) {
    if (scm_exp_is_self_evaluation(pat) ||
        is_literal_id(pat, literals))
        return 0;
    if (IS_IDENTIFIER(pat)) {
        if (pattern_var_exists(*pids, pat))
            syntax_rules_error(pat, "variable used twice in pattern");
        else
            *pids = add_pattern_var(*pids, pat, depth);
        return 1;
    }
    if (pat->type == scm_type_pair || pat == scm_null)
        return check_list_pattern(pat, literals, pids, depth);
    if (pat->type == scm_type_vector)
        return check_vector_pattern(pat, literals, pids, depth);
    else {
        assert(0);
        return 0;
    }
}

static int check_list_template(scm_object *temp, scm_object *pids, long depth, int escape) {
    scm_object *l = temp;
    scm_object *o = scm_car(l);
    scm_object *nexto;
    long max_depth = -1, dep;

    /* list template beginning with ... */
    if (!escape && scm_eq(o, sym_ellipsis)) {
        l = scm_cdr(l);
        if (l->type != scm_type_pair || scm_cdr(l) != scm_null)
            syntax_rules_error(temp, "illegal use of `...` in template");
        return check_template(scm_car(l), pids, depth, 0, 1);
    }

    while (l->type == scm_type_pair) {
        int ellipses = 0;
        l = scm_cdr(l);
        while (l->type == scm_type_pair) {  /* r5rs doesn't support consecutive ... */
            nexto = scm_car(l);
            if (!escape && scm_eq(nexto, sym_ellipsis))
                ellipses++;
            else
                break;
            l = scm_cdr(l);
        }

        dep = check_template(o, pids, depth, ellipses, escape);
        if (dep > max_depth)
            max_depth = dep;
        o = nexto;
    }
    return max_depth;
}

static int check_vector_template(scm_object *temp, scm_object *pids, long depth, int escape) {
    scm_object *o, *nexto;
    long max_depth = -1, dep;
    long i = 0;

    if (temp == scm_empty_vector)
        return max_depth;

    long lent = scm_vector_length(temp);
    o = scm_vector_ref(temp, 0);

    while (i < lent) {
        int ellipses = 0;
        while (i+1 < lent) {
            nexto = scm_vector_ref(temp, i+1);
            if (!escape && scm_eq(nexto, sym_ellipsis))
                ellipses++;
            else
                break;
            ++i;
        }
        dep = check_template(o, pids, depth, ellipses, escape);
        if (dep > max_depth)
            max_depth = dep;
        ++i;
        o = nexto;
    }

    return max_depth;
}

static int check_template(scm_object *temp, scm_object *pids, long depth, long delta, int escape) {
    const char *too_many_ellipses =
    "subtemplates that are followed by `...` must contain at least 1 pattern variable "
    "that occurs in subpattern followed by as many instances of the identifier `...`";

    const char *too_few_ellipses =
        "pattern variables that occur in subpattern followed by one or more "
        "instances of the identifier `...` are allowed only in subtemplates that "
        "are followed by (at least) as many instances of `...`";
    long dep = -1;
    if (scm_exp_is_self_evaluation(temp) ||
        temp == scm_null) {
        /* nop */
    }
    else if (IS_IDENTIFIER(temp)) {
        if (scm_eq(temp, sym_ellipsis))
            syntax_rules_error(temp, "illegal use of `...` in template");

        dep = get_pattern_var_depth(pids, temp);
        if (dep > depth + delta)    /* r5rs: pattern identifier in template must be followed by the same instances of ... as in pattern */
            syntax_rules_error(temp, too_few_ellipses);
    }
    else if (temp->type == scm_type_pair)
        dep = check_list_template(temp, pids, depth + delta, escape);
    else
        dep = check_vector_template(temp, pids, depth + delta, escape);

    if (delta && dep < delta)
        syntax_rules_error(temp, too_many_ellipses);
    return dep - delta;
}

static scm_object *check_pattern(scm_object *pat, scm_object *literals) {
    if (pat->type != scm_type_pair)
        syntax_rules_error(pat, "the outermost <pattern> must be a list-structured form");

    scm_object *pids = scm_null;    /* a list of pairs whose cars are pids and whose cdrs are numbers of following ellipses */

    /* ignore the first element, it's always assumed to be the keyword */
    check_list_pattern(scm_cdr(pat), literals, &pids, 0);
    return pids;
}

void check_syntax_rule(scm_object *rule, scm_object *literals) {
    if (scm_list_length(rule) != 2)
        syntax_rules_error(rule, "<syntax rule> should be of the form (<pattern> <template>)");
    /* check the detail of syntax rule later */
    scm_object *pids = check_pattern(rule_pattern(rule), literals);
    check_template(rule_template(rule), pids, 0, 0, 0);
}


/* pid_binding:
 * (pid #f form) : for pid without following ...
 * (pid n head) : for pid followed by ...
 * @pid:   pattern identifier
 * @#f:    indicates this pid isn't followed by ...
 * @n:     number of occurrence
 * @form:  the matched input form
 * @head:  head of the list of matched input forms. It has recursive structure.
 *         Its elements are in the form of (#f form) or (n head) .
 *         it's in reversed order actually, but it will be reversed again
 *         when expanding.
 */

static scm_object *pid_binding_get_pid(scm_object *pb) {
    return scm_car(pb);
}

static scm_object *pid_binding_get_binding(scm_object *pb) {
    return scm_cdr(pb);
}

static void pid_binding_set_binding(scm_object *pb, scm_object *b) {
    scm_set_cdr(pb, b);
}

static scm_object *binding_get_occurrence(scm_object *b) {
    return scm_car(b);
}

static scm_object *binding_get_form(scm_object *b) {
    return scm_cadr(b);
}

static scm_object *binding_get_head(scm_object *b) {
    return scm_cadr(b);
}

static void binding_set_head(scm_object *b, scm_object *h) {
    scm_set_car(scm_cdr(b), h);
}

//static void binding_append(scm_object *b, scm_object *o) {
//    scm_set_cdr(binding_get_tail(b), o);
//    scm_set_car(scm_cddr(b), o);
//    scm_integer_inc(binding_get_occurrence(b));
//}

static void binding_push(scm_object *b, scm_object *o) {
    scm_object *head = scm_cons(o, binding_get_head(b));
    binding_set_head(b, head);
    scm_integer_inc(binding_get_occurrence(b));
}

static scm_object *binding_pop(scm_object *b) {
    scm_object *head = binding_get_head(b);
    scm_object *o = scm_car(head);
    binding_set_head(b, scm_cdr(head));
    // scm_integer_dec(binding_get_occurrence(b));
    return o;
}

 /*
 * pids: list of pid_bindings
 * can be changed to hash table later */
static scm_object *add_pid_binding(scm_object *pids, scm_object *b) {
    return scm_cons(b, pids);
}

/* two list must have different sets of pattern variables */
static scm_object *combine_pid_bindings(scm_object *pids1, scm_object *pids2) {
    return scm_list_combine(pids1, pids2);
}

/* two list must have the same sets of pattern variables except pids1=scm_null */
static scm_object *merge_pid_bindings(scm_object *pids1, scm_object *pids2) {
    scm_object *pb2;
    scm_object *b;
    scm_object *b2;
    scm_object *pair;
    scm_object *l;
    if (pids1 == scm_null) {
        l = pids2;
        FOREACH_LIST(pb, l) {
            b = pid_binding_get_binding(pb);
            pair = scm_cons(b, scm_null);
            pid_binding_set_binding(pb, scm_list(3, INTEGER(1), pair));
        }
        return pids2;
    }
    else {
        l = pids1;
        FOREACH_LIST(pb, l) {
            pb2 = scm_car(pids2);
            b = pid_binding_get_binding(pb);
            b2 = pid_binding_get_binding(pb2);

            binding_push(b, b2);
        }
        return pids1;
    }
}

static scm_object *lookup_pid_binding(scm_object *pids, scm_object *pid) {
    scm_object *pb;
    while (pids->type == scm_type_pair) {
        pb = scm_car(pids);
        if (scm_eq(pid_binding_get_pid(pb), pid))
            return pid_binding_get_binding(pb);
        pids = scm_cdr(pids);
    }
    return NULL;
}

static scm_object *pid_bind_form(scm_object *pids, scm_object *pid, scm_object *form) {
    return add_pid_binding(pids, scm_list(3, pid, scm_false, form));
}

static scm_object *pid_bind_forms(scm_object *pids, scm_object *pid, int n,
                                  scm_object *head, scm_object *tail) {
    return add_pid_binding(pids, scm_list(4, pid, INTEGER(n), head, tail));
}

static scm_object *get_next_iteration(scm_object *pids) {
    scm_object *l = scm_null;
    scm_object *id, *b, *occur;
    FOREACH_LIST(o, pids) {
        b = pid_binding_get_binding(o);
        occur = binding_get_occurrence(b);
        if (occur == scm_false) {  /* replicate */
            l = scm_cons(o, l);
        }
        else {
            id = pid_binding_get_pid(o);
            l = scm_cons(scm_cons(id, binding_pop(b)), l);
        }
    }
    return l;
}

/* what's having the same lexical binding mean exactly?
 * in the same place of env?  */
static int is_same_binding(scm_object *f, scm_object *p, scm_object *oenv, scm_object *nenv) {
    scm_object *of = scm_env_lookup_var(f, nenv);
    scm_object *op = scm_env_lookup_var(p, oenv);
    /* we compare the address rather than the value here */
    return (of == op) && (of != NULL || scm_eq(f, p));
}

static scm_object *pid_bind_empty_forms(scm_object *p, scm_object *literals) {
    scm_object *pids = scm_null;
    switch (p->type) {
    case scm_type_identifier:
        if (!is_literal_id(p, literals) && !scm_eq(p, sym_ellipsis))
            pids = pid_bind_forms(pids, p, 0, scm_null, scm_null);
        break;
    case scm_type_pair:
        FOREACH_LIST(o, p) {
            if (scm_eq(o, sym_ellipsis))
                continue;
            pids = combine_pid_bindings(pids, pid_bind_empty_forms(o, literals));
        }
        if (p != scm_null)
            pids = combine_pid_bindings(pids, pid_bind_empty_forms(p, literals));
        break;
    case scm_type_vector:
        FOREACH_VECTOR(i, n, p) {
            scm_object *o = scm_vector_ref(p, i);
            if (scm_eq(o, sym_ellipsis))
                continue;
            pids = combine_pid_bindings(pids, pid_bind_empty_forms(o, literals));
        }
        break;
    default:
        break;
    }
    return pids;
}

/*===============  match input form against pattern ================*/

static scm_object *match_vector_ellisis(scm_object *vf, scm_object *p,
    scm_object *literals, scm_object *oenv, scm_object *nenv) {
    scm_object *pidss = scm_null;
    scm_object *pids = NULL;

    FOREACH_VECTOR(i, n, vf) {
        scm_object *f = scm_vector_ref(vf, i);
        pids = match_subpattern(f, p, literals, oenv, nenv, 1);
        if (pids == NULL)
            return NULL;
        pidss = merge_pid_bindings(pidss, pids);
    }

    return pidss;
}

static scm_object *match_vector_pattern(scm_object *vf, scm_object *vp,
    scm_object *literals, scm_object *oenv, scm_object *nenv) {
    if (vf->type != scm_type_vector)
        return NULL;

    scm_object *f, *p;
    scm_object *pidss = scm_null;
    scm_object *pids;
    int has_ellipsis = 0;
    long i = 0;

    long lenp = scm_vector_length(vp);
    long lenf = scm_vector_length(vf);

    if (lenp == 0) {
        if (lenf == 0)
            return pidss;
        else
            return NULL;
    }

    scm_object *lastp = scm_vector_ref(vp, lenp - 1);
    has_ellipsis = scm_eq(lastp, sym_ellipsis);

    if ((!has_ellipsis && lenp != lenf)
        || (has_ellipsis && lenp > lenf + 2)) 
        return NULL;

    while (i < lenp) {
        p = scm_vector_ref(vp, i);

        if (i == lenp - 2 && has_ellipsis) {
            if (i == lenf) {    /* 0 occurrences */
                return combine_pid_bindings(pidss, pid_bind_empty_forms(p, literals));
            }

            pids = match_vector_ellisis(vf, p, literals, oenv, nenv);
            return combine_pid_bindings(pidss, pids);
        }

        f = scm_vector_ref(vf, i);
        pids = match_subpattern(f, p, literals, oenv, nenv, 0);
        if (pids == NULL)
            return NULL;
        pidss = combine_pid_bindings(pidss, pids);

        ++i;
    }

    return pidss;
}

static scm_object *match_sublist_pattern(scm_object *lf, scm_object *lp,
    scm_object *literals, scm_object *oenv, scm_object *nenv) {
    if (lf->type != scm_type_pair && lf != scm_null)
        return NULL;
    return match_list_pattern(lf, lp, literals, oenv, nenv);
}

static scm_object *match_subpattern(scm_object *f, scm_object *p,
    scm_object *literals, scm_object *oenv, scm_object *nenv, int before_ellipsis) {
    switch (p->type) {
    case scm_type_identifier:
        if (!before_ellipsis && is_literal_id(p, literals)) {
            if (!is_same_binding(f, p, oenv, nenv))
                return NULL;
            return scm_null;
        }
        else {  /* pattern identifier matches any input form */
            return pid_bind_form(scm_null, p, f);
        }
    case scm_type_pair:
    case scm_type_null:
        return match_sublist_pattern(f, p, literals, oenv, nenv);
    case scm_type_vector:
        return match_vector_pattern(f, p, literals, oenv, nenv);
    default:
        if (before_ellipsis)
            scm_error("match_subpattern: should not reach here");
        if (scm_equal(f, p))
            return scm_null;
        else
            return NULL;
    }
}

static scm_object *match_list_ellisis(scm_object *lf, scm_object *p,
    scm_object *literals, scm_object *oenv, scm_object *nenv) {
    scm_object *pidss = scm_null;
    scm_object *pids = NULL;

    FOREACH_LIST(f, lf) {
        pids = match_subpattern(f, p, literals, oenv, nenv, 1);
        if (pids == NULL)
            return NULL;
        pidss = merge_pid_bindings(pidss, pids);
    }
    if (lf != scm_null)
        return NULL;    /* F must be a proper list */

    return pidss;
}

static scm_object *match_list_pattern(scm_object *lf, scm_object *lp,
    scm_object *literals, scm_object *oenv, scm_object *nenv) {
    scm_object *f, *p;
    scm_object *pidss = scm_null;
    scm_object *pids;
    scm_object *nextp = NULL;
    int before_ellipsis = 0;
    if (lp->type == scm_type_pair) {
        p = scm_car(lp);
    }
    while (lp->type == scm_type_pair) {
        lp = scm_cdr(lp);
        if (lp->type == scm_type_pair) {
            nextp = scm_car(lp);
            if (scm_eq(nextp, sym_ellipsis))
                before_ellipsis = 1;
        }

        if (lf == scm_null) {
            if (before_ellipsis)    /* 0 occurences */
                return combine_pid_bindings(pidss, pid_bind_empty_forms(p, literals));
            else
                return NULL;
        }

        f = scm_car(lf);
        lf = scm_cdr(lf);
        if (before_ellipsis) {
            pids = match_list_ellisis(lf, p, literals, oenv, nenv);
        }
        else {
            pids = match_subpattern(f, p, literals, oenv, nenv, 0);
        }
        if (pids == NULL)
            return NULL;
        pidss = combine_pid_bindings(pidss, pids);

        if (before_ellipsis)
            return pidss;
        p = nextp;
    }

    if (lp != scm_null) {
        pids = match_subpattern(lf, lp, literals, oenv, nenv, 0);
        if (pids == NULL)
            return NULL;
        pidss = combine_pid_bindings(pidss, pids);
    }

    if (lf != scm_null)
        return NULL;

    return pidss;
}

/* top level pattern.
 * if matches, returns the bindings of pattern identifiers;
 * otherwise, returns NULL */
scm_object *match_pattern(scm_object *lf, scm_object *lp,
                          scm_object *literals, scm_object *oenv, scm_object *nenv) {
    /* skip the keyword */
    lf = scm_cdr(lf);
    lp = scm_cdr(lp);
    return match_list_pattern(lf, lp, literals, oenv, nenv);
}

/*===============  expand the template ================*/

static unsigned int generate_uid() {
    static unsigned int i = 0;
    return ++i;
}

/* TODO: template begin with ... */
static scm_object *expand_subtemplate(scm_object *t, scm_object *pids, scm_object *env,
                                      int uid, int escape) {
    scm_object *b;
    switch (t->type) {
    case scm_type_identifier:
    case scm_type_eidentifier:
        b = lookup_pid_binding(pids, t);
        if (b)
            return binding_get_form(b);
        /* not a pattern variable, return an extended symbol */
        else if (t->type == scm_type_eidentifier)
            return t;
        else
            return scm_esymbol_new(t, uid, env);
    case scm_type_pair:
    case scm_type_null:
        return expand_list_template(t, pids, env, uid, escape);
    case scm_type_vector:
        return expand_vector_template(t, pids, env, uid, escape);
    default:    /* self evaluation */
        return t;
    }
}

static scm_object *gather_pattern_vars(scm_object *t, scm_object *pids) {
    scm_object *head = scm_null;
    switch (t->type) {
    case scm_type_identifier:
    case scm_type_eidentifier:
        if (pattern_var_exists(pids, t))
            return scm_cons(t, head);
        break;
    case scm_type_pair:
        FOREACH_LIST(o, t) {
            head = scm_list_combine(head, gather_pattern_vars(o, pids));
        }
        break;
    case scm_type_vector:
        FOREACH_VECTOR(i, n, t) {
            head = scm_list_combine(head, gather_pattern_vars(scm_vector_ref(t, i), pids));
        }
        break;
    default:
        break;
    }
    return head;
}

static long ellipsis_match_count(scm_object *pids, scm_object *pvars) {
    long n = -1;
    scm_object *b;
    scm_object *occur;
    FOREACH_LIST(o, pvars) {
        b = lookup_pid_binding(pids, o);
        occur = binding_get_occurrence(b);
        if (occur != scm_false) {
            long v = scm_integer_get_val(occur);
            if (n == -1)
                n = v;
            else {
                if (n != v)
                    scm_error("expand_ellipses: incompatible ellipsis match counts for template");
            }
        }
    }
    return n;
}

static scm_object *expand_ellipses(scm_object *t, scm_object *pids,
        scm_object *env, int uid, int ellipses, long *plen, scm_object **ptail) {
    scm_object *head = scm_null;
    scm_object *tail = scm_null;
    scm_object *expanded;
    scm_object *sub_pids;
    ellipses--;
    scm_object *pvars = gather_pattern_vars(t, pids);
    long len = 0;
    *ptail = scm_null;
    /* all pids followed by ... should have the same match count */
    long n = ellipsis_match_count(pids, pvars);
    while (n--) {
        sub_pids = get_next_iteration(pids);
        /* here reverse the list again to resume the order */
        if (ellipses) {
            long len2;    
            expanded = expand_ellipses(t, sub_pids, env, uid, ellipses, &len2, &tail);
            if (head == scm_null) {
                head = expanded;
                *ptail = tail;
            }
            else {
                head = scm_list_combine(expanded, head); /* expanded is in the front */
            }
            len += len2;
        }
        else {
            expanded = expand_subtemplate(t, sub_pids, env, uid, 0);
            if (head == scm_null) {
                head = scm_cons(expanded, head);
                *ptail = head;
            }
            else {
                head = scm_cons(expanded, head);
            }
            ++len;
        }
    }
    *plen = len;
    return head;    
}

static scm_object *expand_list_template(scm_object *temp, scm_object *pids,
                                        scm_object *env, int uid, int escape) {
    scm_object *head = scm_null;
    scm_object *pair, *tail, *new_tail;
    scm_object *t, *nextt, *expanded;

    if (temp->type == scm_type_pair) {
        t = scm_car(temp);
        /* list template beginning with ... */
        if (!escape && scm_eq(t, sym_ellipsis)) {
            return expand_subtemplate(scm_cadr(temp), pids, env, uid, 1);
        }
    }
    while (temp->type == scm_type_pair) {
        int ellipses = 0;
        temp = scm_cdr(temp);
        while (temp->type == scm_type_pair) {   /* r5rs doesn't support consecutive ... */
            nextt = scm_car(temp);
            if (!escape && scm_eq(nextt, sym_ellipsis)) {
                ellipses++;
            }
            else {
                break;
            }
            temp = scm_cdr(temp);
        }

        if (ellipses) {
            long len;
            expanded = expand_ellipses(t, pids, env, uid, ellipses, &len, &new_tail);
            if (head == scm_null) {
                head = expanded;
                tail = new_tail;
            }
            else {
                if (expanded != scm_null) {
                    scm_list_combine(tail, expanded);
                    tail = new_tail;
                }
            }
        }
        else {
            expanded = expand_subtemplate(t, pids, env, uid, escape);
            pair = scm_cons(expanded, scm_null);
            if (head == scm_null) {
                head = pair;
            }
            else {
                scm_set_cdr(tail, pair);
            }
            tail = pair;
        }
        t = nextt;
    }

    return head;
}

static scm_object *expand_vector_template(scm_object *temp, scm_object *pids, scm_object *env, int uid, int escape) {
    scm_object *t, *nextt, *expanded;
    long lent = scm_vector_length(temp);
    if (lent == 0)
        return scm_empty_vector;

    scm_object *v = scm_vector_alloc(lent);
    long lenv = lent;
    long i = 0, j = 0;

    t = scm_vector_ref(temp, i);

    while (i < lent) {
        int ellipses = 0;
        while (++i < lent) {
            nextt = scm_vector_ref(temp, i);
            if (!escape && scm_eq(nextt, sym_ellipsis)) {
                ellipses++;
            }
            else {
                break;
            }
        }

        if (ellipses) {
            long len;
            scm_object *tail;
            expanded = expand_ellipses(t, pids, env, uid, ellipses, &len, &tail);
            if (len != 1 + ellipses) {
                lenv = lenv + len - ellipses - 1;
                v = scm_vector_realloc(v, lenv);
            }

            while (len--) {
                scm_vector_set(v, j++, scm_car(expanded));
                expanded = scm_cdr(expanded);
            }
        }
        else {
            expanded = expand_subtemplate(t, pids, env, uid, escape);
            scm_vector_set(v, j++, expanded);
        }
        t = nextt;
    }

    return v;
}

/* top level template */
static scm_object *expand_template(scm_object *temp, scm_object *pids, scm_object *env) {
    int uid = generate_uid();
    return expand_subtemplate(temp, pids, env, uid, 0);
}

scm_object *transform_macro(scm_object *xformer, scm_object *form, scm_object *env) {
    scm_transformer *x = (scm_transformer *)xformer;
    scm_object *rules = x->rules;
    scm_object *literals = x->literals;
    scm_object *oenv = x->env;
    scm_object *pids;
    FOREACH_LIST(r, rules) {
        pids = match_pattern(form, rule_pattern(r), literals, oenv, env); 
        if (pids) {
            /* extended exp */
            return expand_template(rule_template(r), pids, oenv);
        }
    }

    scm_error_object(form, "%s: bad syntax, no patterns match the input form in: ",
                     scm_variable_get_string(x->kw));

    return NULL;
}

static scm_object_methods xformer_methods = { xformer_free, same_object, same_object };

static int initialized = 0;

int scm_xform_init(void) {
    if (initialized) return 0;

    scm_object_register(scm_type_transformer, &xformer_methods);

    initialized = 1;
    return 0;
}
