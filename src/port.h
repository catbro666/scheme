#ifndef SCHEME_PORT_H
#define SCHEME_PORT_H
#include "object.h"
#include <stdio.h>

typedef struct scm_input_port_st scm_input_port;
typedef struct scm_output_port_st scm_output_port;

scm_object *string_input_port_new(const char *buf, int size);
scm_object *file_input_port_new(FILE *fp);
scm_object *file_input_port_open(const char *name);
/* return -1 when eof */
int scm_input_port_readc(scm_object *port);
int scm_input_port_peekc(scm_object *port);
int scm_input_port_unreadc(scm_object *port, int c);

scm_object *string_output_port_new(char *buf, int size);
scm_object *file_output_port_new(FILE *fp);
scm_object *file_output_port_open(const char *name);
/* return number of bytes written */
int scm_output_port_writec(scm_object *port, char c);

int scm_port_init(void);
int scm_port_init_env(scm_object *env);

#endif /* SCHEME_PORT_H */

