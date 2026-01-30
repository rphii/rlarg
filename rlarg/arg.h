#ifndef RLARG_ARG_H

#include "../rlarg.h"

#include "argx.h"
#include "argx-group.h"
#include "arg-stream.h"

#include <rlso.h>
#include <rlc.h>

enum Arg_Parse_Error_List;

typedef struct Arg_Help_Source {
    size_t i;
    Argx *last;
} Arg_Help_Source;

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
    Arg_Stream stream_in;

    struct {
        bool quit_early;
        bool quit_when_all_valid;
        bool compgen;               /* main trigger for compgen */
        bool compgen_flags;         /* only true has an effect. forces the generation of flags / options */
        bool compgen_done;          /* helps us only printin one single compgen instance */
        bool config; // TODO: should probably rename to config_print; or smth. env_config_print?
        Argx *sources_argx;
        VSo sources_vso;        /* visible vso sources */
        VSo sources_vso_ref;    /* reference vso sources */
        VSo sources_content;    /* content of sources */
        VSo sources_paths;      /* paths to sources */
    } builtin;

    struct {
        Argx *last;     /* generally the last argx encountered while parsing */
        Argx *error;    /* argx to show in case of error */
        bool wanted;    /* if we explicitly want help (e.g. --help) */
        Argx *argx;     /* pointer to the --help argx */
    } help;

} Arg;

#define RLARG_ARG_H
#endif /* RLARG_ARG_H */

