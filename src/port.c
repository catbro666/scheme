#include "port.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct scm_input_port_st {
    short type;
};

typedef int (*readc_fn)(scm_input_port *);
typedef int (*peekc_fn)(scm_input_port *);
typedef int (*unreadc_fn)(scm_input_port *, int c);
typedef void (*iport_free_fn)(scm_input_port *);
#define PORT_CACHE_SIZE 4

enum {
    iport_type_string = 0,
    iport_type_max,
};

static struct iport_callbacks_st {
    readc_fn readc;
    peekc_fn peekc;
    unreadc_fn unreadc;
    iport_free_fn free;
} iport_callbacks[iport_type_max];

/* string_input_port */
struct string_input_port_st {
    scm_input_port base;
    const char *buf;
    size_t size;
    size_t pos;
    char cache[PORT_CACHE_SIZE];
    size_t cache_pos;
};
typedef struct string_input_port_st string_input_port;

static int string_input_port_readc(scm_input_port *port) {
    string_input_port *p = (string_input_port *)port;
    if (p->cache_pos) {
        return p->cache[--(p->cache_pos)];
    }

    if (p->pos == p->size) {
        return -1;
    }
    else {
        return p->buf[p->pos++];
    }
}

static int string_input_port_unreadc(scm_input_port *port, int c) {
    string_input_port *p = (string_input_port *)port;
    (void)c;

    if (p->cache_pos == PORT_CACHE_SIZE) {
        return 1;
    }
    p->cache[p->cache_pos++] = c;
    return 0;
}

static int string_input_port_peekc(scm_input_port *port) {
    int c = string_input_port_readc(port);
    string_input_port_unreadc(port, c);
    return c;
}

static void string_input_port_free(scm_input_port *port) {
    free(port);
    return;
}

static void string_input_port_register() {
    iport_callbacks[iport_type_string].readc = string_input_port_readc;
    iport_callbacks[iport_type_string].peekc = string_input_port_peekc;
    iport_callbacks[iport_type_string].unreadc = string_input_port_unreadc;
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

int scm_input_port_readc(scm_object *port) {
    scm_type type = scm_object_get_type(port);
    scm_input_port *data = (scm_input_port *)scm_object_get_data(port);
    if (type != scm_type_input_port) {
        return -2;  /* TODO: contract violation checking in outer layer */
    }

    return iport_callbacks[data->type].readc(data);
}

int scm_input_port_peekc(scm_object *port) {
    scm_type type = scm_object_get_type(port);
    scm_input_port *data = (scm_input_port *)scm_object_get_data(port);
    if (type != scm_type_input_port) {
        return -2;  /* contract violation */
    }

    return iport_callbacks[data->type].peekc(data);
}

int scm_input_port_unreadc(scm_object *port, int c) {
    scm_type type = scm_object_get_type(port);
    scm_input_port *data = (scm_input_port *)scm_object_get_data(port);
    if (type != scm_type_input_port) {
        return -2;  /* contract violation */
    }

    return iport_callbacks[data->type].unreadc(data, c);
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
