#include "object.h"
#include "port.h"
#include <stdio.h>

int main()
{
    scm_object_env_init();
    scm_port_env_init();

    printf("Hello world\n");
    return 0;
}

