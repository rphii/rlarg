#ifndef RLARG_ARG_H

#include "../rlarg.h"

#include "argx.h"
#include "argx-group.h"
#include "arg-stream.h"

#include <rlso.h>
#include <rlc.h>

typedef struct Arg {
    Argx *c[ARGX_SHORT_COUNT]; /* short options */

    Argx_Groups opts;   /* groups of long options */
    Argx_Group pos;     /* positional arguments */
    Argx_Group env;     /* environment variables */

    int i_pos;          /* index of positional argument parse status */

    T_Argx t_pos;
    T_Argx t_env;
    T_Argx t_opt;       /* root of long options -> delve into groups */

    struct {
        bool compgen;
        Argx *sources_argx;
        So *sources_vso;
        So *sources_vso_ref;
    } builtin;

} Arg;

#define RLARG_ARG_H
#endif /* RLARG_ARG_H */

