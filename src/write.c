#include "write.h"

#include "port.h"
#include "char.h"
#include "number.h"
#include "string.h"
#include "symbol.h"
#include "pair.h"
#include "vector.h"

static int write_raw_string(scm_object *port, char *str) {
    int i = 0;
    char c;
    while((c = str[i])) {
        if (!scm_output_port_writec(port, c)) /* write failed */
            break;
        i++;
    }
    return i;
}

static int write_char(scm_object *port, scm_object *obj) {
    int i = 0;
    i = write_raw_string(port, "#\\");
    if (obj == scm_chars[' ']) {
        i += write_raw_string(port, "space");
    }
    else if (obj == scm_chars['\n']) {
        i += write_raw_string(port, "newline");
    }
    else {
        i += scm_output_port_writec(port, scm_char_get_char(obj));
    }
    return i;
}

static int write_string(scm_object *port, scm_object *obj) {
    int i = 0, j = 0;
    char c;
    i += scm_output_port_writec(port, '"');

    int len = scm_string_length(obj);
    while (j < len) {
        c = scm_string_get_char(obj, j);
        if (c == '\\')
            i += write_raw_string(port, "\\\\");
        else if (c == '"')
            i += write_raw_string(port, "\\\"");
        else
            i += scm_output_port_writec(port, c);

        j++;
    }
    i += scm_output_port_writec(port, '"');

    return i;
}

static int write_pair(scm_object *port, scm_object *obj) {
    int i = 0;
    i += scm_output_port_writec(port, '(');

    while (obj->type == scm_type_pair) {
        i += scm_write(port, scm_car(obj));
        obj = scm_cdr(obj);
        if (obj != scm_null) {
            i += scm_output_port_writec(port, ' ');
        }
    }

    if (obj != scm_null) {
        i += write_raw_string(port, ". ");
        i += scm_write(port, obj);
    }

    i += scm_output_port_writec(port, ')');
    return i;
}

static int write_vector(scm_object *port, scm_object *obj) {
    int i = 0, j = 0;
    i += write_raw_string(port, "#(");
    int n = scm_vector_length(obj);

    while (j < n) {
        i += scm_write(port, scm_vector_ref(obj, j));
        ++j;
        if (j != n) {
            i += scm_output_port_writec(port, ' ');
        }
    }

    i += scm_output_port_writec(port, ')');
    return i;
}

/* return the number of bytes written */
int scm_write(scm_object *port, scm_object *obj) {
    int i = 0;
    switch (obj->type) {
    case scm_type_eof:
        i = write_raw_string(port, "#<eof-object>");
        break;
    case scm_type_null:
        i = write_raw_string(port, "()");
        break;
    case scm_type_true:
        i = write_raw_string(port, "#t");
        break;
    case scm_type_false:
        i = write_raw_string(port, "#f");
        break;
    case scm_type_char:
        i = write_char(port, obj);
        break;
    case scm_type_integer:
    case scm_type_float:
        i = write_raw_string(port, scm_number_to_string(obj, 10));
        break;
    case scm_type_string:
        i = write_string(port, obj);
        break;
    case scm_type_identifier:
        i = write_raw_string(port, scm_symbol_get_string(obj));
        break;
    case scm_type_pair:
        i = write_pair(port, obj);
        break;
    case scm_type_vector:
        i = write_vector(port, obj);
        break;
    case scm_type_input_port:
        i = write_raw_string(port, "#<input-port>");
        break;
    case scm_type_output_port:
        i = write_raw_string(port, "#<output-port>");
        break;
    default:
        break;
    }
    return i;
}
