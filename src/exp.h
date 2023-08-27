#ifndef SCHEME_EXP_H
#define SCHEME_EXP_H
#include "object.h"

extern scm_object *sym_quote;
extern scm_object *sym_quasiquote;
extern scm_object *sym_unquote;
extern scm_object *sym_unquote_splicing;

extern scm_object *sym_lambda;
extern scm_object *sym_if;
extern scm_object *sym_set;
extern scm_object *sym_begin;
extern scm_object *sym_cond;
extern scm_object *sym_and;
extern scm_object *sym_or;
extern scm_object *sym_case;
extern scm_object *sym_let;
extern scm_object *sym_let_astro;
extern scm_object *sym_letrec;
extern scm_object *sym_do;
extern scm_object *sym_delay;

extern scm_object *sym_else;
extern scm_object *sym_define;
extern scm_object *sym_cond_apply;

extern scm_object *sym_let_syntax;
extern scm_object *sym_letrec_syntax;
extern scm_object *sym_define_syntax;
extern scm_object *sym_syntax_rules;
extern scm_object *sym_ellipsis;
extern scm_object *sym_underscore;

extern const int keywords_num;
extern scm_object *keywords[];

int scm_exp_is_self_evaluation(scm_object *exp);
void scm_exp_check_quote(scm_object *exp);
scm_object *scm_exp_get_quote_text(scm_object *exp);
void scm_exp_check_assignment(scm_object *exp);
scm_object *scm_exp_get_assignment_var(scm_object *exp);
scm_object *scm_exp_get_assignment_val(scm_object *exp);
void scm_exp_check_lambda(scm_object *exp);
scm_object *scm_exp_get_lambda_parameters(scm_object *exp);
scm_object *scm_exp_get_lambda_body(scm_object *exp);
void scm_exp_check_definition(scm_object *exp);
scm_object *scm_exp_get_definition_var(scm_object *exp);
scm_object *scm_exp_get_definition_val(scm_object *exp);
void scm_exp_check_if(scm_object *exp);
scm_object *scm_exp_get_if_test(scm_object *exp);
scm_object *scm_exp_get_if_consequent(scm_object *exp);
scm_object *scm_exp_get_if_alternate(scm_object *exp);
void scm_exp_check_begin(scm_object *exp);
scm_object *scm_exp_get_begin_sequence(scm_object *exp);
int scm_exp_is_sequence_last(scm_object *exp);
scm_object *scm_exp_get_sequence_first(scm_object *exp);
scm_object *scm_exp_get_sequence_rest(scm_object *exp);
void scm_exp_check_qq(scm_object *exp);
scm_object *scm_exp_get_qq_exp(scm_object *exp);
void scm_exp_check_unquote(scm_object *exp);
scm_object *scm_exp_get_unquote_exp(scm_object *exp);
void scm_exp_check_unquote_splicing(scm_object *exp);
scm_object *scm_exp_get_unquote_splicing_exp(scm_object *exp);
int scm_exp_is_syntax_or_application(scm_object *exp);
void scm_exp_check_application(scm_object *exp);
void scm_exp_check_syntax(scm_object *exp, const char *kw);
scm_object *scm_exp_get_application_operator(scm_object *exp);
scm_object *scm_exp_get_application_operands(scm_object *exp);
void scm_exp_check_let_syntax(scm_object *exp);
void scm_exp_check_letrec_syntax(scm_object *exp);
void scm_exp_check_define_syntax(scm_object *exp);
scm_object *scm_exp_get_spec_first_rule(scm_object *exp);
int scm_exp_is_spec_empty_rules(scm_object *exp);
scm_object *scm_exp_get_spec_keyword(scm_object *exp);
scm_object *scm_exp_get_spec_literals(scm_object *exp);
scm_object *scm_exp_get_spec_rules(scm_object *exp);
scm_object *scm_exp_get_binding_keyword(scm_object *exp);
scm_object *scm_exp_get_binding_spec(scm_object *exp);
scm_object *scm_exp_get_syntax_bindings(scm_object *exp);
scm_object *scm_exp_get_syntax_first_binding(scm_object *bindings);
scm_object *scm_exp_get_syntax_rest_bindings(scm_object *bindings);
int scm_exp_is_syntax_empty_bindings(scm_object *bindings);
scm_object *scm_exp_get_let_syntax_body(scm_object *exp);
scm_object *scm_exp_get_define_syntax_binding(scm_object *exp);

int scm_exp_init(void);
#endif /* SCHEME_EXP_H */

