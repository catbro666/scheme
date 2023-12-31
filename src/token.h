#ifndef SCHEME_TOKEN_H
#define SCHEME_TOKEN_H
#include "object.h"

#include <stddef.h> /* NULL */

typedef enum {
    scm_token_type_number = 0,
    scm_token_type_string,
    scm_token_type_identifier,
    /* above are dynamic tokens which need to be free'd */

    scm_token_type_eof,
    scm_token_type_true,
    scm_token_type_false,
    scm_token_type_char,
    /* above are simple data that can be an independent object */

    scm_token_type_dot,     /* must be in list */
    scm_token_type_rparen,  /* must has leading lparen */

    /* below are tokens without objects */
    scm_token_type_lparen,
    scm_token_type_sharp_lparen,
    scm_token_type_quote,
    scm_token_type_bquote,
    scm_token_type_comma,
    scm_token_type_comma_at,
    scm_token_type_max,
} scm_token_type;

typedef struct scm_token_st scm_token;

extern scm_token *scm_token_eof;
extern scm_token *scm_token_true;
extern scm_token *scm_token_false;
extern scm_token *scm_token_chars[];
extern scm_token *scm_token_lparen;
extern scm_token *scm_token_rparen;
extern scm_token *scm_token_sharp_lparen;
extern scm_token *scm_token_quote;
extern scm_token *scm_token_bquote;
extern scm_token *scm_token_comma;
extern scm_token *scm_token_comma_at;
extern scm_token *scm_token_dot;

scm_token *scm_token_read(scm_object *port);
short scm_token_get_type(scm_token *tok);
scm_object *scm_token_get_obj(scm_token *tok);
void scm_token_free(scm_token *tok);

int scm_token_init(void);

#endif /* SCHEME_TOKEN_H */

