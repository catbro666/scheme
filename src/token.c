#include "token.h"
#include "err.h"
#include "port.h"
#include "char.h"
#include "string.h"
#include "symbol.h"
#include "number.h"

#include <string.h>
#include <ctype.h> /* isspace */
#include <stdlib.h>

struct scm_token_st {
    short type;
    scm_object *obj;
};

static scm_token scm_token_eof_arr[1]          = {{scm_token_type_eof, NULL}};
static scm_token scm_token_true_arr[1]         = {{scm_token_type_true, NULL}};
static scm_token scm_token_false_arr[1]        = {{scm_token_type_false, NULL}};
static scm_token scm_token_lparen_arr[1]       = {{scm_token_type_lparen, NULL}};
static scm_token scm_token_rparen_arr[1]       = {{scm_token_type_rparen, NULL}};
static scm_token scm_token_sharp_lparen_arr[1] = {{scm_token_type_sharp_lparen, NULL}};
static scm_token scm_token_quote_arr[1]        = {{scm_token_type_quote, NULL}};
static scm_token scm_token_bquote_arr[1]       = {{scm_token_type_bquote, NULL}};
static scm_token scm_token_comma_arr[1]        = {{scm_token_type_comma, NULL}};
static scm_token scm_token_comma_at_arr[1]     = {{scm_token_type_comma_at, NULL}};
static scm_token scm_token_dot_arr[1]          = {{scm_token_type_dot, NULL}};
#define CHAR_NUM 128
static scm_token scm_token_chars_arr[CHAR_NUM];
scm_token *scm_token_chars[CHAR_NUM];
scm_token *scm_token_eof          = scm_token_eof_arr;
scm_token *scm_token_true         = scm_token_true_arr;
scm_token *scm_token_false        = scm_token_false_arr;
scm_token *scm_token_lparen       = scm_token_lparen_arr;
scm_token *scm_token_rparen       = scm_token_rparen_arr;
scm_token *scm_token_sharp_lparen = scm_token_sharp_lparen_arr;
scm_token *scm_token_quote        = scm_token_quote_arr;
scm_token *scm_token_bquote       = scm_token_bquote_arr;
scm_token *scm_token_comma        = scm_token_comma_arr;
scm_token *scm_token_comma_at     = scm_token_comma_at_arr;
scm_token *scm_token_dot       = scm_token_dot_arr;

static scm_token *scm_token_new(short type, scm_object *obj);

static int is_newline(int c) {
    return c == '\n';
}

static int is_bin_digit(int c) {
    return c == '0' || c =='1';
}

static int is_oct_digit(int c) {
    return c >= '0' && c <='7';
}

static int is_hex_digit(int c) {
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int is_radix_digit(int c, int radix) {
    switch (radix) {
    case 2:
        return is_bin_digit(c);
    case 8:
        return is_oct_digit(c);
    case 16:
        return is_hex_digit(c);
    default:    /* 10 */
        return isdigit(c);
    }
}

static int is_delimiter(int c) {
    if (isspace(c)) {
        return 1;
    }
    switch (c) {
    case '(':
    case ')':
    case '"':
    case ';':
        return 1;
    }

    return 0;
}

static int is_eof(int c) {
    return c == -1;
}

static int is_delimiter_or_eof(int c) {
    return is_eof(c) || is_delimiter(c);
}

static int is_special_initial(int c) {
    switch (c) {
    case '!': case '$': case '%': case '&':
    case '*': case '/': case ':': case '<':
    case '=': case '>': case '?': case '^':
    case '_': case '~':
        return 1;
    }

    return 0;
}

static int is_initial(int c) {
    return isalpha(c) || is_special_initial(c);
}

static int is_special_subsequent(int c) {
    switch (c) {
    case '+': case '-': case '.': case '@':
        return 1;
    }
    
    return 0;
}

static int is_subsequent(int c) {
    return is_initial(c) || isdigit(c) || is_special_subsequent(c);
}

static void skip_comment(scm_object *port) {
    int c;
    while (1) {
        c = scm_input_port_readc(port);
        if (is_newline(c)) {
            return;
        }
    }
}

static void skip_space_comment(scm_object *port) {
    int c;
    while (1) {
        c = scm_input_port_peekc(port);
        if (is_eof(c) || !isspace(c)) {
            if (c == ';') {
                skip_comment(port);
            }
            else {
                return;
            }
        }

        scm_input_port_readc(port);
    }
}

static int is_exponent_marker(int c) {
    switch (c) {
    case 'e': case 's': case 'f': case 'd': case 'l':
    case 'E': case 'S': case 'F': case 'D': case 'L':
        return 1;
    }
    return 0;
}

static scm_token *make_peculiar_identifier(const char *str, int len) {
    scm_object *obj = scm_symbol_new(str, len);
    if (!obj) {
        return NULL;
    }

    return scm_token_new(scm_token_type_identifier, obj);
}

static const char *char_error_fmt = "lexer: bad character constant `#\\%s`";
static const char *identifier_error_fmt = "lexer: bad identifier `%s`";
static const char *string_error_fmt = "lexer: bad string `%s`";
static const char *number_error_prefix = "lexer: bad number";

scm_token *read_char(scm_object *port) {
    scm_token *tok = NULL;
    int c;
    int i = 0;
    int size = 7;
    char *str = malloc(size + 1);

    c = scm_input_port_readc(port);
    if (is_eof(c))
        goto err;

    while (1) {
      str[i++] = (char)c;

      c = scm_input_port_peekc(port);
      if (is_delimiter_or_eof(c))
          break;

      scm_input_port_readc(port);

      if (i == size) {
          size *= 2;
          str = realloc(str, size + 1);
      }
    }

    str[i] = '\0';
    if (i == 1) {
        tok = scm_token_chars[(int)str[0]];
    }
    else if (i == 5 || i == 7) {
        if (!strcmp(str, "space")) {
            tok = scm_token_chars[' '];
        }
        else if (!strcmp(str, "newline")) {
            tok = scm_token_chars['\n']; /* consider \r when displaying */
        }
        else {
            goto err;
        }
    } 
    else {
        goto err;
    }

    free(str);
    return tok;
err:
    str[i] = '\0';
    scm_error_free(free, str, char_error_fmt, str);
    return NULL; /* impossible to reach here */
}

static scm_token *read_identifier(scm_object *port, int c) {
    int size = 32, i = 0;
    char *str = malloc(size + 1);
    scm_object *obj = NULL;

    str[i++] = (char)c;
    while (1) {
        c = scm_input_port_peekc(port);

        if (is_delimiter_or_eof(c)) {
            break;
        }

        scm_input_port_readc(port);

        if (i == size) {
            size *= 2;
            str = realloc(str, size + 1);
        }

        str[i++] = (char)c;

        if (!is_subsequent(c)) {
            goto err;
        }
    }

    str[i] = '\0';
    obj = scm_symbol_new(str, i);
    free(str);
    return scm_token_new(scm_token_type_identifier, obj);
err:
    str[i] = '\0';
    scm_error_free(free, str, identifier_error_fmt, str);
    return NULL; /* impossible to reach here */
}

static scm_token *read_string(scm_object *port) {
    int size = 0, i = 0, c;
    char *str = NULL;

    while (1) {
        c = scm_input_port_readc(port);

        if (c == '"' || is_eof(c)) {
          break;
        }

        if (i == size) {
            if (size == 0) {
                size = 32;
                str = malloc(size + 2); /* +2 for the following error case */
            }
            else {
                size *= 2;
                str = realloc(str, size + 2);
            }
        }

        if (c == '\\') {
            c = scm_input_port_readc(port);
            if (c != '"' && c != '\\') {
                str[i++] = '\\';
                str[i++] = c;
                goto err;   /* bad syntax */
            }
        }

        str[i++] = (char)c;
    }
    
    if (i) {
        str[i] = '\0';
    }

    if (is_eof(c)) {
        goto err;    /* bad syntax: no ending " */
    }

    /* XXX: shrink the string size? */
    return scm_token_new(scm_token_type_string, scm_string_new(str, i));
err:
    if (str)
        scm_error_free(free, str, string_error_fmt, str);
    else
        scm_error(string_error_fmt, "");
    return NULL;
}

static void number_error(char *buf, char *num, int len, char *msg) {
    if (len) num[len] = '\0';
    scm_error_free(free, num, "%s `%s%s`;\n%s",
                   number_error_prefix, buf, num ? num : "", msg);
}

static void number_error_char(char *buf, char *num, int len, char *msg, char c) {
    if (len) num[len] = '\0';
    scm_error_free(free, num, "%s `%s%s`;\n%s `%c`",
                   number_error_prefix, buf, num ? num : "", msg, c);
}

/* TODO: support complex, rational */
/* complex = real + real i
 * real = integer/integer | float */

static scm_token *read_number(scm_object *port) {
    /* number containing point, exponent or sharp is inexact by default */
    int radix = -1, exactness = -1, is_float = 0;
    int c, c2, i = 2, j = 0;
    int num_size = 0;
    char *num = NULL;
    int has_point = 0, has_sharp = 0, has_exponent = 0, has_digit = 0;
    scm_object *obj;
    scm_token *tok = NULL;
    c = scm_input_port_peekc(port);
    char buf[7] = {0};

    /* prefix */
    while (i > 0) {
        if (c != '#') {
            break;
        }

        scm_input_port_readc(port);
        buf[j++] = c;

        c2 = scm_input_port_peekc(port);
        buf[j++] = c2;
        switch (c2) {
        case 'i': case 'I':
            if (exactness != -1) {
                number_error(buf, num, 0, "duplicate exactness specification");
            }
            exactness = 0;
            break;
        case 'e': case 'E':
            if (exactness != -1) {
                number_error(buf, num, 0, "duplicate exactness specification");
            }
            exactness = 1;
            break;
        case 'b': case 'B':
            if (radix != -1) {
                number_error(buf, num, 0, "duplicate radix specification");
            }
            radix = 2;
            break;
        case 'o': case 'O':
            if (radix != -1) {
                number_error(buf, num, 0, "duplicate radix specification");
            }
            radix = 8;
            break;
        case 'd': case 'D':
            if (radix != -1) {
                number_error(buf, num, 0, "duplicate radix specification");
            }
            radix = 10;
            break;
        case 'x': case 'X':
            if (radix != -1) {
                number_error(buf, num, 0, "duplicate radix specification");
            }
            radix = 16;
            break;
        default:
            number_error_char(buf, num, 0, "bad `#` indicator", c2);
        }
        scm_input_port_readc(port);

        c = scm_input_port_peekc(port);
        --i;
    }

    if (radix == -1) {
        radix = 10;
    }

    i = 0;
    j = i;  /* save the beginning index */
    while (1) {
        if (is_delimiter_or_eof(c) ||
            is_exponent_marker(c)) {
            break;
        }

        scm_input_port_readc(port);

        if (i == num_size) {
            if (num_size == 0) {
                num_size = 16;
                num = malloc(num_size + 1);
            }
            else {
                num_size *= 2;
                num = realloc(num, num_size + 1);
            }
        }

        num[i++] = (char)c;

        /* sign must be at the beginning */
        if (i == (j+1) && (c == '+' || c == '-')) {
        }
        else if (c == '.') {
            if (radix != 10 || has_point) {
                number_error(buf, num, i, "`.` can only appear in decimal radix"
                             " and at most once");
            }
            has_point = 1;
        }
        else if (c == '#') {
            if (radix != 10 || !has_digit) {
                number_error(buf, num, i, "`#`s can only appear at the end of a"
                             " decimal number with at least one leading digit");
            }
            num[i-1] = '0'; /* change to 0 */
            has_sharp = 1;
        }
        else if (is_radix_digit(c, radix)) {
            if (has_sharp) {
                number_error(buf, num, i, "`.` can only appear in decimal radix"
                             " and at most once");
            }
            has_digit = 1;
        }
        else {
            number_error_char(buf, num, i, "bad digit", c);
        }

        c = scm_input_port_peekc(port);
    }

    if (!has_digit) {
        number_error(buf, num, i, "no digit");
    }

    /* suffix */
    if (is_exponent_marker(c)) {
        if (radix != 10) {
            number_error(buf, num, i, "numbers containing exponents must be in "
                         "decimal radix");
        }
        has_digit = 0;
        j = i;  /* save the beginning index */

        while (1) {
            if (is_delimiter_or_eof(c)) {
                break;
            }
            scm_input_port_readc(port);

            if (i == num_size) {
                if (num_size == 0) {
                    num = malloc(num_size + 1);
                }
                else {
                    num = realloc(num, num_size + 1);
                }
            }

            num[i++] = (char)c;

            if (i == (j+1)) {   /* the exponent marker */
            }
            /* sign must be at the beginning */
            else if (i == (j+2) && (c == '+' || c == '-')) {
            }
            else if (isdigit(c)) {
               has_digit = 1;
            }
            else {
                number_error_char(buf, num, i, "bad digit", c);
            }

            c = scm_input_port_peekc(port);
        }

        if (!has_digit) {
            number_error(buf, num, i, "no digit");
        }
        has_exponent = 1;
    }
    num[i] = '\0';

    if (has_point || has_exponent) {
        is_float = 1;
    }

    /* default exactness */
    if (exactness == -1) {
        if (is_float || has_sharp) {
            exactness = 0;
        }
        else {
            exactness = 1;
        }
    }

    if (exactness && is_float) {
        obj = scm_number_new_integer_from_float(num);
    }
    else if (exactness && !is_float) {
        obj = scm_number_new_integer(num, radix);
    }
    else if (is_float) {
        obj = scm_number_new_float(num);
    }
    else {
        obj = scm_number_new_float_from_integer(num, radix);
    }

    if (!obj) {
        number_error(buf, num, i, "out of range");
    }

    tok = scm_token_new(scm_token_type_number, obj);

    free(num);
    return tok;
}

/* read a token from port
 * return NULL when error */
scm_token *scm_token_read(scm_object *port) {
    int c, c2;
    char buf[5] = {0};

    skip_space_comment(port);

    c = scm_input_port_readc(port);

    if (is_eof(c)) {
        return scm_token_eof;
    }

    switch (c) {
    case '\'':
        return scm_token_quote;
    case '`':
        return scm_token_bquote;
    case ',':
        c2 = scm_input_port_peekc(port);
        if (c2 == '@') {
            scm_input_port_readc(port);
            return scm_token_comma_at;
        }
        return scm_token_comma;
    case '(':
        return scm_token_lparen; 
    case ')':
        return scm_token_rparen; 
    case '"':
        return read_string(port);
    case '#':
        c2 = scm_input_port_peekc(port);

        switch (c2) {
        case '(':
            scm_input_port_readc(port);
            return scm_token_sharp_lparen;
        case 't': case 'T':
            scm_input_port_readc(port);
            return scm_token_true;
        case 'f': case 'F':
            scm_input_port_readc(port);
            return scm_token_false;
        case '\\':
            scm_input_port_readc(port);
            return read_char(port);
        default:
            scm_input_port_unreadc(port, c);
            return read_number(port);
        }
    default:
        if (isdigit(c)) {
            scm_input_port_unreadc(port, c);
            return read_number(port);
        }
        else if (is_initial(c)) {
            return read_identifier(port, c);
        }

        switch (c) {
        case '+':
        case '-':
            c2 = scm_input_port_peekc(port);
            if (is_delimiter_or_eof(c2)) {
                scm_input_port_readc(port);

                if (c == '+') {
                    return make_peculiar_identifier("+", 1);
                }
                else {
                    return make_peculiar_identifier("-", 1);
                }
            }
            else {
                scm_input_port_unreadc(port, c);
                return read_number(port);
            }
        case '.':
            c2 = scm_input_port_peekc(port);
            if (is_delimiter_or_eof(c2)) {
                return scm_token_dot;
            }
            else if (isdigit(c2)) {
                scm_input_port_unreadc(port, c);
                return read_number(port);
            }
            buf[0] = (char)c;
            buf[1] = (char)c2;
            scm_input_port_readc(port);
            c = scm_input_port_readc(port);
            buf[2] = (char)c;
            if (c2 != '.' || c != '.') {
                goto err;
            }
            c = scm_input_port_peekc(port);
            buf[4] = (char)c;
            if (!is_delimiter_or_eof(c)) {
                goto err;
            }
            return make_peculiar_identifier("...", 3);
        default:
            buf[0] = (char)c;
            goto err;
        }
    }
err:
    scm_error(identifier_error_fmt, buf);
    return NULL;    /* impossible to reach here */
}

static scm_token *scm_token_new(short type, scm_object *obj) {
    scm_token *tok = malloc(sizeof(scm_token));
    if (!tok) {
        return NULL;    /* malloc failure */
    }

    tok->type = type;
    tok->obj = obj;

    return tok;
}

short scm_token_get_type(scm_token *tok) {
    return tok->type;
}

scm_object *scm_token_get_obj(scm_token *tok) {
    return tok->obj;
}

void scm_token_free(scm_token *tok) {
    /* skip the static tokens */
    if (tok->type >= scm_token_type_eof) {
        return;
    }
    free(tok);
}

static int initialized = 0;

int scm_token_init(void) {
    int i = 0;

    if (initialized)
        return 0;

    scm_token_eof_arr[0].obj = scm_eof;
    scm_token_true_arr[0].obj = scm_true;
    scm_token_false_arr[0].obj = scm_false;
    scm_token_dot_arr[0].obj = scm_dot;
    scm_token_rparen_arr[0].obj = scm_rparen;
    for (i = 0; i < CHAR_NUM; ++i) {
        scm_token_chars_arr[i].type = scm_token_type_char;
        scm_token_chars_arr[i].obj = scm_chars[i];
        scm_token_chars[i] = scm_token_chars_arr + i;
    }

    initialized = 1;
    return 0;
}
