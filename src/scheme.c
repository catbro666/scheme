#include "object.h"
#include "err.h"
#include "char.h"
#include "port.h"
#include "string.h"
#include "symbol.h"
#include "number.h"
#include "token.h"
#include "pair.h"
#include "vector.h"
#include "env.h"
#include "exp.h"
#include "read.h"
#include "write.h"
#include "eval.h"
#include "xform.h"

#include <stdio.h>

int main()
{
    scm_object_init();
    scm_char_init();
    scm_port_init();
    scm_string_init();
    scm_symbol_init();
    scm_number_init();
    scm_token_init();
    scm_pair_init();
    scm_vector_init();
    scm_exp_init();
    scm_proc_init();
    scm_eval_init();
    scm_xform_init();

    scm_object *env = scm_global_env();
    const char *prompt = "> ";
    const char *welcome = "Welcome to the Scheme World!\n"
        "hit CTRL+D to quit the program.\n";

    scm_object *iport = default_iport;
    scm_object *oport = default_oport;
    scm_output_port_puts(oport, welcome);
    while (1) {
        scm_output_port_puts(oport, prompt);
        SCM_TRY {
            scm_object *exp = scm_read(iport);
            if (exp == scm_eof) {
                return 0;
            }
            scm_object *res = scm_eval(exp, env);
            if (res != scm_void) {
                scm_write(oport, res);
                scm_newline(oport);
            }
        } SCM_CATCH {
            char *err = scm_error_msg();
            scm_output_port_puts(oport, err);
            scm_newline(oport);
        } SCM_END_TRY;
    }

    return 0;
}

