#include "port.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct scm_input_port_st {
    short type;
};

typedef scm_object* (*readc_fn)(scm_input_port *);
typedef scm_object* (*peekc_fn)(scm_input_port *);
typedef void (*iport_free_fn)(scm_input_port *);

enum {
    iport_type_string = 0,
    iport_type_max,
};

static struct iport_callbacks_st {
    readc_fn readc;
    peekc_fn peekc;
    iport_free_fn free;
} iport_callbacks[iport_type_max];

/* string_input_port */
struct string_input_port_st {
    scm_input_port base;
    const char *buf;
    size_t size;
    size_t pos;
};
typedef struct string_input_port_st string_input_port;

static scm_object *string_input_port_readc(scm_input_port *port) {
    string_input_port *p = (string_input_port *)port;
    if (p->pos == p->size) {
        return scm_eof;
    }
    else {
        return scm_chars[(unsigned)p->buf[p->pos++]];
    }
}

static scm_object *string_input_port_peekc(scm_input_port *port) {
    string_input_port *p = (string_input_port *)port;
    if (p->pos == p->size) {
        return scm_eof;
    }
    else {
        return scm_chars[(unsigned)p->buf[p->pos]];
    }
}

static void string_input_port_free(scm_input_port *port) {
    free(port);
    return;
}

static void string_input_port_register() {
    iport_callbacks[iport_type_string].readc = string_input_port_readc;
    iport_callbacks[iport_type_string].peekc = string_input_port_peekc;
    iport_callbacks[iport_type_string].free = string_input_port_free;
    return;
}


/* public */

/* use the external buffer */
scm_object *string_input_port_new(const char *buf, int size) {
    if (buf == NULL) {
        return NULL;
    }

    if (size == -1) {
        size = strlen(buf);
    }

    string_input_port *port = calloc(1, sizeof(string_input_port));
    if (port == NULL) {
        return NULL;
    }

    port->base.type = iport_type_string;
    port->buf = buf;
    port->size = size;
    port->pos = 0;

    scm_object *obj = scm_object_new(scm_type_input_port, (intptr_t)port);
    if (obj == NULL) {
        free(port);
        return NULL;
    }

    return obj;
}

scm_object *scm_input_port_readc(scm_object *port) {
    scm_type type = scm_object_get_type(port);
    scm_input_port *data = (scm_input_port *)scm_object_get_data(port);
    if (type != scm_type_input_port) {
        return NULL;
    }

    return iport_callbacks[data->type].readc(data);
}

scm_object *scm_input_port_peekc(scm_object *port) {
    scm_type type = scm_object_get_type(port);
    scm_input_port *data = (scm_input_port *)scm_object_get_data(port);
    if (type != scm_type_input_port) {
        return NULL;
    }

    return iport_callbacks[data->type].peekc(data);
}

static void scm_input_port_free(void *port) {
    scm_input_port *p = (scm_input_port *)port;
    iport_callbacks[p->type].free(p);
    return;
}

static int initialized = 0;

int scm_port_env_init(void) {
    if (initialized) return 0;

    string_input_port_register();

    scm_object_register(scm_type_input_port, scm_input_port_free);

    initialized = 1;
    return 0;
}
