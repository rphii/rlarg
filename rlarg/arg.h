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

    T_Argx t_pos;       /* root of positional arguments */
    T_Argx t_env;       /* root of environment variables */
    T_Argx t_opt;       /* root of long options -> delve into groups */

    Argx_Callback_Queue *queue;   /* any callback that we encountered */

    struct {
        bool quit_early;
        bool quit_when_all_valid;
        bool compgen;
        bool config;
        Argx *sources_argx;
        So *sources_vso;
        So *sources_vso_ref;
    } builtin;

    struct {
        bool wanted;
        bool error;
        bool given;
        Argx *last;
        Argx *argx;
    } help;

} Arg;

#define RLARG_ARG_H
#endif /* RLARG_ARG_H */

