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

/* 0 denotes duplicating for the innermost ...
 * 1 denotes duplicating for the outermost ... */
int ellipsis_duplicate_mode = 0;

#define rule_pattern scm_car
#define rule_template scm_cadr

static int check_subpattern(scm_object *pat, scm_object *literals, scm_object **pids, int depth);
static int check_subtemplate(scm_object *temp, scm_object *pids, long depth, long delta, int escape);
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
        if (same_id(o, sym_ellipsis))
            syntax_rules_error(pat, "the `...` must be after subpattern including pattern identifiers");
    }
    while (l->type == scm_type_pair) {
        l = scm_cdr(l);
        if (l->type == scm_type_pair) {
            nexto = scm_car(l);
            if (same_id(nexto, sym_ellipsis)) {
                if (scm_cdr(l) != scm_null)
                    syntax_rules_error(pat, "the `...` must be at the end of a <pattern> in the sequence form");
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
        if (same_id(l, sym_ellipsis))
            syntax_rules_error(l, "the `...` must be at the end of a <pattern> in the sequence form");
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
    if (len > 0 && same_id(scm_vector_ref(pat, 0), sym_ellipsis))
        syntax_rules_error(pat, "the `...` must be after subpattern including pattern identifiers");

    FOREACH_VECTOR(i, len, pat) {
        if (i < (len - 1)) {
            if (same_id(scm_vector_ref(pat, i+1), sym_ellipsis)) {
                if (i != (len - 2))
                    syntax_rules_error(pat, "the `...` must be at the end of a <pattern> in the sequence form");
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
        if (scm_eq(scm_car(o), pid)) /* seems unnecessary to use same_id */
            return 1;
    }
    return 0;
}

static scm_object *add_pattern_var(scm_object *pids, scm_object *pid, long depth) {
    return scm_cons(scm_cons(pid, INTEGER(depth)), pids);
}

static long get_pattern_var_depth(scm_object *pids, scm_object *pid) {
    scm_object *o = scm_assq(pid, pids);
    if (o == scm_false)
        return -1;
    else
        return scm_integer_get_val(scm_cdr(o));
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
    if (!escape && same_id(o, sym_ellipsis)) {
        l = scm_cdr(l);
        if (l->type != scm_type_pair || scm_cdr(l) != scm_null)
            syntax_rules_error(temp, "illegal use of `...` in template");
        return check_subtemplate(scm_car(l), pids, depth, 0, 1);
    }

    while (l->type == scm_type_pair) {
        int ellipses = 0;
        l = scm_cdr(l);
        while (l->type == scm_type_pair) {  /* r5rs doesn't support consecutive ... */
            nexto = scm_car(l);
            if (!escape && same_id(nexto, sym_ellipsis))
                ellipses++;
            else
                break;
            l = scm_cdr(l);
        }

        dep = check_subtemplate(o, pids, depth, ellipses, escape);
        if (dep > max_depth)
            max_depth = dep;
        o = nexto;
    }

    if (l != scm_null) {
        dep = check_subtemplate(l, pids, depth, 0, escape);
        if (dep > max_depth)
            max_depth = dep;
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
            if (!escape && same_id(nexto, sym_ellipsis))
                ellipses++;
            else
                break;
            ++i;
        }
        dep = check_subtemplate(o, pids, depth, ellipses, escape);
        if (dep > max_depth)
            max_depth = dep;
        ++i;
        o = nexto;
    }

    return max_depth;
}

static int check_subtemplate(scm_object *temp, scm_object *pids, long depth, long delta, int escape) {
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
        if (same_id(temp, sym_ellipsis))
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

/* export this function for test purpose */
int check_template(scm_object *temp, scm_object *pids) {
    return check_subtemplate(temp, pids, 0, 0, 0);
}

/* export this function for test purpose */
scm_object *check_pattern(scm_object *pat, scm_object *literals) {
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
    check_template(rule_template(rule), pids);
}


/* pid_binding:
 * (pid 0 form) : for pid without following ...
 * (pid n vector) : for pid followed by ...
 * @pid:   pattern identifier
 * @n:     the depth of ...
 * @form:  the matched input form
 * @vector:  vector of matched input forms. It is recursive structure.
 *         Its elements are in the form of (0 form) or (n vector) .
 */

static scm_object *pid_binding_get_pid(scm_object *pb) {
    return scm_car(pb);
}

scm_object *pid_binding_get_binding(scm_object *pb) {
    return scm_cdr(pb);
}

static long binding_get_depth(scm_object *b) {
    return scm_integer_get_val(scm_car(b));
}

static scm_object *binding_get_form(scm_object *b) {
    return scm_cadr(b);
}

static scm_object *binding_get_vect(scm_object *b) {
    return scm_cadr(b);
}

long pid_binding_get_depth(scm_object *pb) {
    return binding_get_depth(pid_binding_get_binding(pb));
}

scm_object *pid_binding_get_form(scm_object *pb) {
    return binding_get_form(pid_binding_get_binding(pb));
}

scm_object *pid_binding_get_vect(scm_object *pb) {
    return binding_get_vect(pid_binding_get_binding(pb));
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

scm_object *lookup_pid_binding(scm_object *pids, scm_object *pid) {
    scm_object *pb;
    while (pids->type == scm_type_pair) {
        pb = scm_car(pids);
        if (scm_eq(pid_binding_get_pid(pb), pid))
            return pb;
        pids = scm_cdr(pids);
    }
    return scm_false;
}

static scm_object *pid_bind_form(scm_object *pids, scm_object *pid, scm_object *form) {
    return add_pid_binding(pids, scm_list(3, pid, INTEGER(0), form));
}

static scm_object *pid_bind_forms(scm_object *pids, scm_object *pid, long dep, long len) {
    scm_object *vec = scm_empty_vector;
    if (len > 0)
        vec = scm_vector_alloc(len);
    return add_pid_binding(pids, scm_list(3, pid, INTEGER(dep), vec));
}

static scm_object *bind_pids_ellipsis(scm_object *p, scm_object *literals, long dep, long len) {
    scm_object *head = scm_null;
    long ellipsis = 0;
    switch (p->type) {
    case scm_type_identifier:
    case scm_type_eidentifier:
        if (is_pattern_id(p, literals))
            return pid_bind_forms(head, p, dep, len);
        break;
    case scm_type_pair:
        FOREACH_LIST(o, p) {
            if (IS_PAIR(p) && same_id(scm_car(p), sym_ellipsis))
                ellipsis = 1;
            head = scm_list_combine(head, bind_pids_ellipsis(o, literals, dep + ellipsis, len));
            if (ellipsis)
                break;
        }
        break;
    case scm_type_vector:
        FOREACH_VECTOR(i, n, p) {
            if (i == n -2 && same_id(scm_vector_ref(p, n-1), sym_ellipsis))
                ellipsis = 1;
            head = scm_list_combine(head, bind_pids_ellipsis(scm_vector_ref(p, i), literals, dep + ellipsis, len));
            if (ellipsis)
                break;
        }
        break;
    default:
        break;
    }
    return head;
}

static void insert_match_instance(scm_object *pidss, scm_object *pids, long index) {
    scm_object *id;
    scm_object *b, *vec;
    FOREACH_LIST(pb, pids) {
        id = pid_binding_get_pid(pb);
        b = pid_binding_get_binding(pb);
        vec = pid_binding_get_vect(scm_assq(id, pidss));
        scm_vector_set(vec, index, b);
    }
}

static scm_object *get_next_iteration(scm_object *pids, long i) {
    scm_object *l = scm_null;
    scm_object *id, *b, *vec;
    long dep;
    FOREACH_LIST(o, pids) {
        b = pid_binding_get_binding(o);
        dep = binding_get_depth(b);
        if (dep == 0) {  /* replicate for innermost ... */
            l = scm_cons(o, l);
        }
        else {
            id = pid_binding_get_pid(o);
            vec = binding_get_vect(b);
            l = scm_cons(scm_cons(id, scm_vector_ref(vec, i)), l);
        }
    }
    return l;
}

static scm_object *get_next_iteration1(scm_object *pids, long i, long max_dep) {
    scm_object *l = scm_null;
    scm_object *id, *b, *vec;
    long dep;
    FOREACH_LIST(o, pids) {
        b = pid_binding_get_binding(o);
        dep = binding_get_depth(b);
        if (dep < max_dep) {  /* replicate for outermost ... */
            l = scm_cons(o, l);
        }
        else {
            id = pid_binding_get_pid(o);
            vec = binding_get_vect(b);
            l = scm_cons(scm_cons(id, scm_vector_ref(vec, i)), l);
        }
    }
    return l;
}

/* what's having the same lexical binding mean exactly?
 * in the same place of env?  */
static int is_same_binding(scm_object *f, scm_object *p, scm_object *oenv, scm_object *nenv) {
    scm_object *of = scm_env_lookup_var(nenv, f);
    scm_object *op = scm_env_lookup_var(oenv, p);
    /* we compare the address rather than the value here */
    return (of == op) && (of != NULL || scm_eq(f, p));
}


/*===============  match input form against pattern ================*/
static scm_object *match_vector_ellisis(scm_object *vf, scm_object *p,
    scm_object *literals, scm_object *oenv, scm_object *nenv, long start) {
    scm_object *pids = NULL;
    long n = scm_vector_length(vf);
    scm_object *pidss = bind_pids_ellipsis(p, literals, 1, n - start);
    long j = 0;

    for (long i = start; i < n; ++i) {
        scm_object *f = scm_vector_ref(vf, i);
        pids = match_subpattern(f, p, literals, oenv, nenv, 1);
        if (pids == NULL)
            return NULL;
        insert_match_instance(pidss, pids, j);
        ++j;
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
    has_ellipsis = same_id(lastp, sym_ellipsis);

    if ((!has_ellipsis && lenp != lenf)
        || (has_ellipsis && lenp > lenf + 2)) 
        return NULL;

    while (i < lenp) {
        p = scm_vector_ref(vp, i);

        if (i == lenp - 2 && has_ellipsis) {
            pids = match_vector_ellisis(vf, p, literals, oenv, nenv, i);
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
    case scm_type_eidentifier:
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
    long len = scm_list_length(lf);
    if (len < 0)
        return NULL;    /* F must be a proper list */
    scm_object *pidss = bind_pids_ellipsis(p, literals, 1, len);
    scm_object *pids;
    long i = 0;

    FOREACH_LIST(f, lf) {
        pids = match_subpattern(f, p, literals, oenv, nenv, 1);
        if (pids == NULL)
            return NULL;
        insert_match_instance(pidss, pids, i);
        ++i;
    }

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
            if (same_id(nextp, sym_ellipsis))
                before_ellipsis = 1;
        }

        if (before_ellipsis) {
            pids = match_list_ellisis(lf, p, literals, oenv, nenv);
        }
        else {
            if (!IS_PAIR(lf))
                return NULL;
            f = scm_car(lf);
            pids = match_subpattern(f, p, literals, oenv, nenv, 0);
        }
        if (pids == NULL)
            return NULL;
        pidss = combine_pid_bindings(pidss, pids);

        if (before_ellipsis)
            return pidss;
        lf = scm_cdr(lf);
        p = nextp;
    }

    if (lp != scm_null) {
        pids = match_subpattern(lf, lp, literals, oenv, nenv, 0);
        if (pids == NULL)
            return NULL;
        return combine_pid_bindings(pidss, pids);
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

    if (lp == scm_null || IS_PAIR(lp))
        return match_list_pattern(lf, lp, literals, oenv, nenv);
    else
        return match_subpattern(lf, lp, literals, oenv, nenv, 0);
}

/*===============  expand the template ================*/

static unsigned int generate_uid() {
    static unsigned int i = 0;
    return ++i;
}

static scm_object *expand_subtemplate(scm_object *t, scm_object *pids, scm_object *env,
                                      int uid, int escape) {
    scm_object *pb = NULL;
    switch (t->type) {
    case scm_type_identifier:
    case scm_type_eidentifier:
        pb = lookup_pid_binding(pids, t);
        if (pb != scm_false)
            return pid_binding_get_form(pb);
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

static scm_object *get_sub_pids(scm_object *t, scm_object *pids, long *max_dep) {
    scm_object *head = scm_null;
    scm_object *pb;
    long dep = -1;
    switch (t->type) {
    case scm_type_identifier:
    case scm_type_eidentifier:
        pb = lookup_pid_binding(pids, t);
        if (pb != scm_false) {
            *max_dep = pid_binding_get_depth(pb);
            return scm_cons(pb, head);
        }
        break;
    case scm_type_pair:
        FOREACH_LIST(o, t) {
            head = scm_list_combine(head, get_sub_pids(o, pids, &dep));
            if (dep > *max_dep)
                *max_dep = dep;
        }
        break;
    case scm_type_vector:
        FOREACH_VECTOR(i, n, t) {
            head = scm_list_combine(head, get_sub_pids(scm_vector_ref(t, i), pids, &dep));
            if (dep > *max_dep)
                *max_dep = dep;
        }
        break;
    default:
        break;
    }
    return head;
}

static long ellipsis_match_count(scm_object *pids) {
    long n = -1;
    scm_object *b;
    long len, dep;
    FOREACH_LIST(pb, pids) {
        b = pid_binding_get_binding(pb);
        dep = binding_get_depth(b);
        /* duplicate for the innermost ..., skip dep == 0 */
        if (dep) {
            len = scm_vector_length(binding_get_vect(b));
            if (n == -1)
                n = len;
            else {
                if (n != len)
                    scm_error("expand_ellipses: incompatible ellipsis match counts for template");
            }
        }
    }
    return n;
}

static long ellipsis_match_count1(scm_object *pids, long max_dep) {
    long n = -1;
    scm_object *b;
    long len, dep;

    FOREACH_LIST(pb, pids) {
        b = pid_binding_get_binding(pb);
        dep = binding_get_depth(b);
        /* duplicate for the outermost ..., skip dep < max_dep */
        if (dep == max_dep) {
            len = scm_vector_length(binding_get_vect(b));
            if (n == -1)
                n = len;
            else {
                if (n != len)
                    scm_error("expand_ellipses: incompatible ellipsis match counts for template");
            }
        }
    }
    return n;
}

static scm_object *expand_ellipses(scm_object *t, scm_object *pids,
        scm_object *env, int uid, int ellipses, long *plen, scm_object **ptail, long max_dep) {
    scm_object *head = scm_null;
    scm_object *tail = scm_null, *new_tail = scm_null;
    scm_object *expanded, *sub_pids, *pair;
    ellipses--;
    long len = 0;
    *ptail = scm_null;
    long n;
    /* all pids followed by ... should have the same match count */
    if (ellipsis_duplicate_mode)
        n = ellipsis_match_count1(pids, max_dep);
    else
        n = ellipsis_match_count(pids);

    for (long i = 0; i < n; ++i) {
        if (ellipsis_duplicate_mode)
            sub_pids = get_next_iteration1(pids, i, max_dep);
        else
            sub_pids = get_next_iteration(pids, i);

        if (ellipses) {
            long len2;    
            expanded = expand_ellipses(t, sub_pids, env, uid, ellipses, &len2, &new_tail, max_dep - 1);
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
            len += len2;
        }
        else {
            expanded = expand_subtemplate(t, sub_pids, env, uid, 0);
            pair = scm_cons(expanded, scm_null);
            if (head == scm_null) {
                head = pair;
            }
            else {
                scm_set_cdr(tail, pair);
            }
            tail = pair;
            ++len;
        }
    }
    *plen = len;
    *ptail = tail;
    return head;    
}

static scm_object *expand_list_template(scm_object *temp, scm_object *pids,
                                        scm_object *env, int uid, int escape) {
    scm_object *head = scm_null;
    scm_object *pair, *tail, *new_tail;
    scm_object *t, *nextt, *expanded, *sub_pids;
    long max_dep = -1;

    if (temp->type == scm_type_pair) {
        t = scm_car(temp);
        /* list template beginning with ... */
        if (!escape && same_id(t, sym_ellipsis)) {
            return expand_subtemplate(scm_cadr(temp), pids, env, uid, 1);
        }
    }
    while (temp->type == scm_type_pair) {
        int ellipses = 0;
        temp = scm_cdr(temp);
        while (temp->type == scm_type_pair) {   /* r5rs doesn't support consecutive ... */
            nextt = scm_car(temp);
            if (!escape && same_id(nextt, sym_ellipsis)) {
                ellipses++;
            }
            else {
                break;
            }
            temp = scm_cdr(temp);
        }

        sub_pids = get_sub_pids(t, pids, &max_dep);

        if (ellipses) {
            long len;
            expanded = expand_ellipses(t, sub_pids, env, uid, ellipses, &len, &new_tail, max_dep);
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
            expanded = expand_subtemplate(t, sub_pids, env, uid, escape);
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
    scm_object *t, *nextt, *expanded, *sub_pids;
    long max_dep = -1;
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
            if (!escape && same_id(nextt, sym_ellipsis)) {
                ellipses++;
            }
            else {
                break;
            }
        }

        sub_pids = get_sub_pids(t, pids, &max_dep);

        if (ellipses) {
            long len;
            scm_object *tail;
            expanded = expand_ellipses(t, sub_pids, env, uid, ellipses, &len, &tail, max_dep);
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
            expanded = expand_subtemplate(t, sub_pids, env, uid, escape);
            scm_vector_set(v, j++, expanded);
        }
        t = nextt;
    }

    return v;
}

/* top level template */
scm_object *expand_template(scm_object *temp, scm_object *pids, scm_object *env) {
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
