#ifndef RLARG_ARG_H

#include "../rlarg.h"

#include "argx.h"
#include "argx-group.h"
#include "arg-stream.h"

#include <rlso.h>
#include <rlc.h>

#define ARG_SPACING_HINT_WRAP               40
#define ARG_SPACING_HINT_ALTERNATE          10

#define ARG_SPACING_DESCRIPTION_DEFAULT     40
//#define ARG_SPACING_DESCRIPTION_ALTERNATE   12
#define ARG_SPACING_DESCRIPTION_ALTERNATE   40

#define ARG_SPACING_VALUE_WRAP_ARRAY        42
#define ARG_SPACING_VALUE_WRAP_DELIM        40

enum Arg_Parse_Error_List;

typedef struct Arg_Config {
    So program;
    So description;
    So epilog;
    struct {
        int max;    // max width
        int desc;   // spacing until description
        int c;      // spacing until short option
        int opt;    // spacing until long option
    } bounds;
} Arg_Config;

typedef struct Arg_Help_Source {
    size_t i;
    Argx *last;
} Arg_Help_Source;

typedef struct Arg_Rice {
    So_Fx program;
    So_Fx program_delim;
    So_Fx program_desc;
    So_Fx group;
    So_Fx group_delim;
    So_Fx pos;
    So_Fx c;
    So_Fx opt;
    So_Fx env;
    So_Fx desc;
    So_Fx subopt;
    So_Fx subopt_delim;
    So_Fx enum_unset;
    So_Fx enum_set;
    So_Fx enum_delim;
    So_Fx flag_unset;
    So_Fx flag_set;
    So_Fx flag_delim;
    So_Fx hint;
    So_Fx hint_delim;
    So_Fx val;
    So_Fx val_delim;
    So_Fx sw;
    So_Fx sw_delim;
    So_Fx sequence;
    So_Fx sequence_delim;
} Arg_Rice;

typedef struct ArgPrint {
    So_Align_Cache p_al2;
    So_Align whitespace;
    bool compgen_nfirst;
} Arg_Print;

typedef struct Arg {
    Argx *c[ARGX_SHORT_COUNT]; /* short options */
    Arg_Print print;
    Arg_Config config;

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
        bool quit_when_all_parsed;
        bool compgen;               /* main trigger for compgen */
        bool compgen_flags;         /* only true has an effect. forces the generation of flags / options TODO I think is unused... */
        bool compgen_done;          /* helps us only printin one single compgen instance */
        bool config_print_selected; // TODO: should probably rename to config_print; or smth. env_config_print?
        bool config_use_builtin;    /* instruct to generate groups of all options right before arg_parse ... */
        Arg_Builtin_Color_List color;  /* control color mode */
        bool color_off;             /* need this bool due to So_Fx */
        Argx *sources_argx;
        VSo sources_vso;        /* visible vso sources */
        VSo sources_content;    /* content of sources */
        VSo sources_paths;      /* paths to sources */
    } builtin;

    struct {
        Argx *last;     /* generally the last argx encountered while parsing */
        Argx *error;    /* argx to show in case of error */
        bool wanted;    /* if we explicitly want help (e.g. --help) */
        Argx *argx;     /* pointer to the --help argx */
        VSo sub;
    } help;

    Arg_Rice rice;

} Arg;

#define RLARG_ARG_H
#endif /* RLARG_ARG_H */

