#include "port.h"
#include "sys.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>

scm_object *default_iport = NULL;
scm_object *default_oport = NULL;

/* input port */
#define PORT_CACHE_SIZE 4

struct scm_input_port_st {
    scm_object base;
    short type;
    char cache[PORT_CACHE_SIZE];
    size_t cache_pos;
};

typedef int (*readc_fn)(scm_input_port *);

enum {
    iport_type_string = 0,
    iport_type_file,
    iport_type_max,
};

static struct iport_callbacks_st {
    readc_fn readc;
    scm_object_free_fn free;
    scm_eq_fn eqv;
} iport_callbacks[iport_type_max];

/* output port */
struct scm_output_port_st {
    scm_object base;
    short type;
};

typedef int (*writec_fn)(scm_output_port *, char);

enum {
    oport_type_string = 0,
    oport_type_file,
    oport_type_max,
};

static struct oport_callbacks_st {
    writec_fn writec;
    scm_object_free_fn free;
    scm_eq_fn eqv;
} oport_callbacks[oport_type_max];

/* specific types of input/output port */
/* string_input_port */
struct string_input_port_st {
    scm_input_port base;
    const char *buf;
    size_t size;
    size_t pos;
};
typedef struct string_input_port_st string_input_port;

static int string_input_port_readc(scm_input_port *port) {
    if (port->cache_pos) {
        return port->cache[--(port->cache_pos)];
    }

    string_input_port *p = (string_input_port *)port;
    if (p->pos == p->size) {
        return -1;  /* eof */
    }
    else {
        return p->buf[p->pos++];
    }
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
    iport_callbacks[iport_type_string].free = string_input_port_free;
    iport_callbacks[iport_type_string].eqv = string_input_port_eqv;
    return;
}

/* file_input_port */
struct file_input_port_st {
    scm_input_port base;
    FILE *fp;
};
typedef struct file_input_port_st file_input_port;

static int file_input_port_readc(scm_input_port *port) {
    if (port->cache_pos) {
        return port->cache[--(port->cache_pos)];
    }

    file_input_port *p = (file_input_port *)port;
    int c = fgetc(p->fp);
    if (c == EOF)
        return -1;
    else
        return c;
}

static void file_input_port_free(scm_object *port) {
    file_input_port *p = (file_input_port *)port;
    fclose(p->fp);
    free(p);
    return;
}

static int file_input_port_eqv(scm_object *o1, scm_object *o2) {
    return o1 == o2;
}

static void file_input_port_register() {
    iport_callbacks[iport_type_file].readc = file_input_port_readc;
    iport_callbacks[iport_type_file].free = file_input_port_free;
    iport_callbacks[iport_type_file].eqv = file_input_port_eqv;
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

/* file_output_port */
struct file_output_port_st {
    scm_output_port base;
    FILE *fp;
};
typedef struct file_output_port_st file_output_port;

static int file_output_port_writec(scm_output_port *port, char c) {
    file_output_port *p = (file_output_port *)port;
    int ret = fputc(c, p->fp);
    if (ret == EOF)
        return 0;
    else
        return 1;
}

static void file_output_port_free(scm_object *port) {
    file_output_port *p = (file_output_port *)port;
    fclose(p->fp);
    free(p);
    return;
}

static int file_output_port_eqv(scm_object *o1, scm_object *o2) {
    return o1 == o2;
}

static void file_output_port_register() {
    oport_callbacks[oport_type_file].writec = file_output_port_writec;
    oport_callbacks[oport_type_file].free = file_output_port_free;
    oport_callbacks[oport_type_file].eqv = file_output_port_eqv;
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

scm_object *file_input_port_new(FILE *fp) {
    file_input_port *port = calloc(1, sizeof(file_input_port));
    if (port == NULL) {
        return NULL;
    }

    port->base.base.type = scm_type_input_port;
    port->base.type = iport_type_file;
    port->fp = fp;

    return (scm_object *)port;
}

static FILE *open_input_file(const char *name) {
    FILE *fp = fopen(name, "r");
    if (!fp) {
        scm_sys_err("open-input-file: can't open input file\n"
                    "name: %s\n", name);
    }

    return fp;
}

scm_object *file_input_port_open(const char *name) {
    FILE *fp = open_input_file(name);

    return file_input_port_new(fp);
}

int scm_input_port_readc(scm_object *obj) {
    if (obj->type != scm_type_input_port) {
        return -2;  /* TODO: contract violation checking in outer layer */
    }

    scm_input_port *port = (scm_input_port *)obj;

    return iport_callbacks[port->type].readc(port);
}

int scm_input_port_unreadc(scm_object *obj, int c) {
    if (obj->type != scm_type_input_port) {
        return -2;  /* contract violation */
    }

    scm_input_port *p = (scm_input_port *)obj;

    if (p->cache_pos == PORT_CACHE_SIZE) {
        return 1;
    }
    p->cache[p->cache_pos++] = c;
    return 0;
}

int scm_input_port_peekc(scm_object *obj) {
    if (obj->type != scm_type_input_port) {
        return -2;  /* contract violation */
    }

    scm_input_port *port = (scm_input_port *)obj;
    int c = iport_callbacks[port->type].readc(port);
    if (c != -1) {
        scm_input_port_unreadc(obj, c);
    }
    return c;
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

scm_object *file_output_port_new(FILE *fp) {
    file_output_port *port = calloc(1, sizeof(file_output_port));
    if (port == NULL) {
        return NULL;
    }

    port->base.base.type = scm_type_output_port;
    port->base.type = oport_type_file;
    port->fp = fp;

    return (scm_object *)port;
}

static FILE *open_output_file(const char *name) {
    FILE *fp = fopen(name, "w");
    if (!fp) {
        scm_sys_err("open-output-file: can't open output file\n"
                    "name: %s\n", name);
    }

    return fp;
}

scm_object *file_output_port_open(const char *name) {
    FILE *fp = open_output_file(name);

    return file_output_port_new(fp);
}


int scm_output_port_writec(scm_object *obj, char c) {
    scm_output_port *port = (scm_output_port *)obj;

    return oport_callbacks[port->type].writec(port, c);
}

int scm_output_port_puts(scm_object *obj, const char *p) {
    scm_output_port *port = (scm_output_port *)obj;
    int i = 0;
    char c;
    while ((c = *(p++))) {
        i += oport_callbacks[port->type].writec(port, c);
    }

    return i;
}

int scm_newline(scm_object *obj) {
    scm_output_port *port = (scm_output_port *)obj;
    return oport_callbacks[port->type].writec(port, '\n');
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
    file_input_port_register();
    file_output_port_register();

    default_iport = file_input_port_new(stdin);
    /* we use stderr here, because stdout is line buffering
     * but we need to output the prompt at the start of a command line */
    default_oport = file_output_port_new(stderr);

    scm_object_register(scm_type_input_port, &input_methods);
    scm_object_register(scm_type_output_port, &output_methods);

    initialized = 1;
    return 0;
}

int scm_port_init_env(scm_object *env) {
    (void)env;
    return 0;
}
