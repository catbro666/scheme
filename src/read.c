#include "read.h"
#include "err.h"
#include "port.h"
#include "symbol.h"
#include "token.h"
#include "pair.h"
#include "vector.h"

static scm_object *scm_object_read_ex(scm_object *port, int in_seq);
static scm_object *read_list(scm_object *port);
static scm_object *read_vector(scm_object *port);
static scm_object *read_quote(scm_object *port);
static scm_object *read_quasiquote(scm_object *port);
static scm_object *read_unquote(scm_object *port);
static scm_object *read_unquote_splicing(scm_object *port);

static void read_list_error(scm_object *head, char *msg) {
    scm_object_free(head);
    scm_error("parser: %s", msg);
}

static scm_object *read_list(scm_object *port) {
    scm_object *head = scm_null;
    scm_object *tail = NULL;
    scm_object *pair = NULL;
    scm_object *obj= NULL;
    int expect_last = 0, expect_rparen = 0;

    while (1) {
        obj = scm_object_read_ex(port, 1);
        if (!obj || obj == scm_eof) {
            read_list_error(head, "missing right parenthese");
        }
        if (obj == scm_rparen) {
            if (expect_last) {  /* dot must be followed with an object */
                read_list_error(head, "unexpected `)`");
            }
            break;
        }
        
        if (expect_rparen) {
            read_list_error(head, "illegal use of `.`");
        }

        if (obj == scm_dot) {
            /* dot can't be at the beginning of list or after dot */
            if (head == scm_null || expect_last) {
                read_list_error(head, "illegal use of `.`");
            }
            expect_last = 1;
            continue;
        }
        else {
            if (expect_last) {
                expect_last = 0;
                expect_rparen = 1;
                scm_set_cdr(tail, obj);
            }
            else {
                pair = scm_cons(obj, scm_null);
                if (head == scm_null) {
                    head = pair;
                }
                else {
                    scm_set_cdr(tail, pair);
                }
                tail = pair;
            }
        }
    }

    return head;
}

static scm_object *read_vector(scm_object *port) {
    scm_object *vec = scm_empty_vector;
    scm_object *obj = NULL;

    while (1) {
        SCM_TRY {
            obj = scm_object_read_ex(port, 2); /* handle . */
        } SCM_CATCH {
            scm_object_free(vec);
            SCM_THROW;
        } SCM_END_TRY;

        if (!obj || obj == scm_eof) {
            scm_object_free(vec);
            scm_error("parser: missing right parenthese");
        }
        if (obj == scm_rparen) {
            break;
        }
        vec = scm_vector_insert(vec, obj);
    }

    return vec;
}

static scm_object *read_quote(scm_object *port) {
    return scm_list(2, scm_symbol_new("quote", 5), scm_object_read(port));
}

static scm_object *read_quasiquote(scm_object *port) {
    return scm_list(2, scm_symbol_new("quasiquote", 10), scm_object_read(port));
}

static scm_object *read_unquote(scm_object *port) {
    return scm_list(2, scm_symbol_new("unquote", 7), scm_object_read(port));
}

static scm_object *read_unquote_splicing(scm_object *port) {
    return scm_list(2, scm_symbol_new("unquote-splicing", 16), scm_object_read(port));
}

/* read an object from port
 * @in_seq: denote if the current object is in list or vector
 *          0 means not, 1 means in list, 2 means in vector */
static scm_object *scm_object_read_ex(scm_object *port, int in_seq) {
    scm_token *tok = scm_token_read(port);
    short type = scm_token_get_type(tok);
    scm_object *obj = scm_token_get_obj(tok);
    scm_token_free(tok);

    /* simple datum */
    if (type <= scm_token_type_rparen) {
        if (type == scm_token_type_dot && in_seq != 1) {
            scm_error("parser: illegal use of `.`");
        }
        if (type == scm_token_type_rparen && !in_seq) {
            scm_error("parser: unexpected `)`");
        }
        return obj;
    }
    /* compound datum */
    else {
        switch (type) {
        case scm_token_type_lparen:
            obj = read_list(port);
            break;
        case scm_token_type_sharp_lparen:
            obj = read_vector(port);
            break;
        /* we will check the quote stuff later.
         * read function doesn't responsible for checking syntax
         * and also these tokens are only abbreviations */
        case scm_token_type_quote:
            obj = read_quote(port);
            break;
        case scm_token_type_bquote:
            obj = read_quasiquote(port);
            break;
        case scm_token_type_comma:  /* must within quasiquote */
            obj = read_unquote(port);
            break;
        case scm_token_type_comma_at: /* must within a list or vector of quasiquote */
            obj = read_unquote_splicing(port);
            break;
         /* impossible to reach here */
        }
    }

    return obj;
}

scm_object *scm_object_read(scm_object *port) {
    return scm_object_read_ex(port, 0);
}
