#include "port.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* input port */
struct scm_input_port_st {
    scm_object base;
    short type;
};

typedef int (*readc_fn)(scm_input_port *);
typedef int (*peekc_fn)(scm_input_port *);
typedef int (*unreadc_fn)(scm_input_port *, int c);

#define PORT_CACHE_SIZE 4

enum {
    iport_type_string = 0,
    iport_type_max,
};

static struct iport_callbacks_st {
    readc_fn readc;
    peekc_fn peekc;
    unreadc_fn unreadc;
    scm_object_free_fn free;
    scm_object_eq_fn eqv;
} iport_callbacks[iport_type_max];

/* output port */
struct scm_output_port_st {
    scm_object base;
    short type;
};

typedef int (*writec_fn)(scm_output_port *, char);

enum {
    oport_type_string = 0,
    oport_type_max,
};

static struct oport_callbacks_st {
    writec_fn writec;
    scm_object_free_fn free;
    scm_object_eq_fn eqv;
} oport_callbacks[oport_type_max];

/* specific types of input/output port */
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
        return -1;  /* eof */
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
    if (c != -1) {
        string_input_port_unreadc(port, c);
    }
    return c;
}

static void string_input_port_free(scm_object *port) {
    free(port);
    return;
}

static int string_input_port_eqv(scm_object *o1, scm_object *o2) {
    return o1 == o2;
}

static void string_input_port_register() {
    iport_callbacks[iport_type_string].readc = string_input_port_readc;
    iport_callbacks[iport_type_string].peekc = string_input_port_peekc;
    iport_callbacks[iport_type_string].unreadc = string_input_port_unreadc;
    iport_callbacks[iport_type_string].free = string_input_port_free;
    iport_callbacks[iport_type_string].eqv = string_input_port_eqv;
    return;
}

/* string_output_port */
struct string_output_port_st {
    scm_output_port base;
    char *buf;
    size_t size;
    size_t pos;
};
typedef struct string_output_port_st string_output_port;

static int string_output_port_writec(scm_output_port *port, char c) {
    string_output_port *p = (string_output_port *)port;
    if (p->pos == p->size) {
        return 0;   /* buf is already full */
    }

    p->buf[p->pos++] = c;
    return 1;
}

static void string_output_port_free(scm_object *port) {
    free(port);
    return;
}

static int string_output_port_eqv(scm_object *o1, scm_object *o2) {
    return o1 == o2;
}

static void string_output_port_register() {
    oport_callbacks[oport_type_string].writec = string_output_port_writec;
    oport_callbacks[oport_type_string].free = string_output_port_free;
    oport_callbacks[oport_type_string].eqv = string_output_port_eqv;
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

    port->base.base.type = scm_type_input_port;
    port->base.type = iport_type_string;
    port->buf = buf;
    port->size = size;
    port->pos = 0;

    return (scm_object *)port;
}

int scm_input_port_readc(scm_object *obj) {
    if (obj->type != scm_type_input_port) {
        return -2;  /* TODO: contract violation checking in outer layer */
    }

    scm_input_port *port = (scm_input_port *)obj;

    return iport_callbacks[port->type].readc(port);
}

int scm_input_port_peekc(scm_object *obj) {
    if (obj->type != scm_type_input_port) {
        return -2;  /* contract violation */
    }

    scm_input_port *port = (scm_input_port *)obj;
    return iport_callbacks[port->type].peekc(port);
}

int scm_input_port_unreadc(scm_object *obj, int c) {
    if (obj->type != scm_type_input_port) {
        return -2;  /* contract violation */
    }

    scm_input_port *port = (scm_input_port *)obj;

    return iport_callbacks[port->type].unreadc(port, c);
}

static void iport_free(scm_object *obj) {
    scm_input_port *port = (scm_input_port *)obj;
    iport_callbacks[port->type].free(obj);
    return;
}

static int iport_eqv(scm_object *o1, scm_object *o2) {
    scm_input_port *p1 = (scm_input_port *)o1;
    scm_input_port *p2 = (scm_input_port *)o2;
    if (p1->type != p2->type)
        return 0;
    return iport_callbacks[p1->type].eqv(o1, o2);
}

scm_object *string_output_port_new(char *buf, int size) {
    if (buf == NULL) {
        return NULL;
    }

    if (size == -1) {
        size = strlen(buf);
    }

    string_output_port *port = malloc(sizeof(string_output_port));
    if (port == NULL) {
        return NULL;
    }

    port->base.base.type = scm_type_output_port;
    port->base.type = oport_type_string;
    port->buf = buf;
    port->size = size;
    port->pos = 0;

    return (scm_object *)port;
}

int scm_output_port_writec(scm_object *obj, char c) {
    scm_output_port *port = (scm_output_port *)obj;

    return oport_callbacks[port->type].writec(port, c);
}

static void oport_free(scm_object *obj) {
    scm_output_port *port = (scm_output_port *)obj;
    oport_callbacks[port->type].free(obj);
    return;
}

static int oport_eqv(scm_object *o1, scm_object *o2) {
    scm_output_port *p1 = (scm_output_port *)o1;
    scm_output_port *p2 = (scm_output_port *)o2;
    if (p1->type != p2->type)
        return 0;
    return oport_callbacks[p1->type].eqv(o1, o2);
}

static scm_object_methods input_methods = { iport_free, iport_eqv, iport_eqv };
static scm_object_methods output_methods = { oport_free, oport_eqv, oport_eqv };

static int initialized = 0;

int scm_port_init(void) {
    if (initialized) return 0;

    string_input_port_register();
    string_output_port_register();

    scm_object_register(scm_type_input_port, &input_methods);
    scm_object_register(scm_type_output_port, &output_methods);

    initialized = 1;
    return 0;
}
