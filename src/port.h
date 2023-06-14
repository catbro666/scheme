#ifndef SCHEME_PORT_H
#define SCHEME_PORT_H
#include "object.h"

typedef struct scm_input_port_st scm_input_port;

scm_object *string_input_port_new(const char *buf, int size);

scm_object *scm_input_port_readc(scm_object *port);
scm_object *scm_input_port_peekc(scm_object *port);

int scm_port_env_init(void);

#endif /* SCHEME_PORT_H */

