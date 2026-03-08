#ifndef RLARG_ARG_X_H

#include <rlso.h>
#include <rlc.h>
#include "../rlarg.h"
#include "argx-hint.h"
#include "argx-so.h"
#include "argx-attr.h"
#include "argx-callback.h"

#define ARGX_SHORT_MIN      ('!')
#define ARGX_SHORT_MAX      ('~')
#define ARGX_SHORT_COUNT    (ARGX_SHORT_MAX - ARGX_SHORT_MIN)

typedef struct Arg_Stream_Source Arg_Stream_Source;

typedef enum {
    ARGX_TYPE_NONE,
    ARGX_TYPE_REST,
    ARGX_TYPE_INT,
    ARGX_TYPE_SIZE,
    ARGX_TYPE_BOOL,
    ARGX_TYPE_STRING,
    ARGX_TYPE_URI,
    ARGX_TYPE_GROUP,
    ARGX_TYPE_ENUM,
    ARGX_TYPE_FLAG,
    ARGX_TYPE_COLOR,
    ARGX_TYPE_SWITCH,
    /* keep above */
    ARGX_TYPE__COUNT,
} Argx_Type_List;

typedef union Argx_Value_Union {
    void *any;
    int *i;
    int **vi;
    ssize_t *z;
    ssize_t **vz;
    bool *b;
    bool **vb;
    So *so;
    VSo *vso;
    Color *c;
    Color **vc;
    struct Argx_Switch *sw;
} Argx_Value_Union;

#if 0
#define ARGX_SOURCE_REFVAL  (Arg_Stream_Source){ .path = so("refval") }
#define ARGX_SOURCE_STDIN   (Arg_Stream_Source){ .path = so("stdin") }
#define ARGX_SOURCE_ENVVARS (Arg_Stream_Source){ .path = so("envvars") }
#define ARGX_SOURCE_NONE    (Arg_Stream_Source){ .path = so("none") }
#else
#define ARGX_SOURCE_NONE    (Arg_Stream_Source){ .id = ARG_STREAM_SOURCE_NONE }
#define ARGX_SOURCE_STDIN   (Arg_Stream_Source){ .id = ARG_STREAM_SOURCE_STDIN }
#define ARGX_SOURCE_REFVAL  (Arg_Stream_Source){ .id = ARG_STREAM_SOURCE_REFVAL }
#define ARGX_SOURCE_SOURCE  (Arg_Stream_Source){ .id = ARG_STREAM_SOURCE_SOURCE }
#define ARGX_SOURCE_ENVVARS (Arg_Stream_Source){ .id = ARG_STREAM_SOURCE_ENVVARS }
#endif

typedef struct Argx_Switch {
    struct Argx *argx;
    Argx_Value_Union val;
} Argx_Switch;

typedef struct Argx {
    Argx_Value_Union val;  /* parsed value */
    Argx_Value_Union ref;  /* reference / default value (refval) */
    Argx_Hint hint;
    Argx_Type_List id;
    Arg_Stream_Source *sources;  /* from where the value gets set -- should be an array if is_array, else, a single value. optional with a line number (some.config:123) */
    struct Argx_Group *group_p; /* always set to parent group */
    struct Argx_Group *group_s; /* only set if id == ARGX_GROUP */
    Argx_Callback callback;
    Argx_Attr attr;
    So desc;
    So opt;
    char c;
} Argx, *V_Argx;

LUT_INCLUDE(T_Argx, t_argx, So, BY_VAL, Argx, BY_VAL);

void v_argx_free(V_Argx *vargs);

void argx_fmt_help(So *out, Argx *argx);
void argx_fmt_config(So *out, Argx *argx);

void arg_update_color_off(struct Arg *arg); // TODO: move this somewhere else (also source)

bool argx_flag_is_any_source_set(Argx *argx);
bool argx_is_configurable(Argx *argx);
bool argx_is_subgroup_of_root(Argx *argx, struct Argx_Group *group);
//bool argx_is_multiline_config(Argx *argx);

void argx_builtin_env_config(struct Arg *arg);

#define RLARG_ARG_X_H
#endif /* RLARG_ARG_X_H */

