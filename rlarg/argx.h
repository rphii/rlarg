#ifndef RLARG_ARG_X_H

#include <rlso.h>
#include <rlc.h>
#include "argx-hint.h"

#define ARG_X_SHORT_COUNT   ('~' - '!')

typedef enum {
    ARGX_TYPE_NONE,
    ARGX_TYPE_REST,
    ARGX_TYPE_INT,
    ARGX_TYPE_SIZE,
    ARGX_TYPE_BOOL,
    ARGX_TYPE_STRING,
    ARGX_TYPE_URI,
    ARGX_TYPE_GROUP,
} Argx_Type_List;

typedef enum {
    ARGX_PRIORITY_WHEN_VALID,
    ARGX_PRIORITY_IMMEDIATELY,
} Argx_Priority_List;

typedef union Argx_Value_Union {
    int i;
    int *vi;
    ssize_t z;
    ssize_t *vz;
    bool b;
    bool *vb;
    So so;
    VSo vso;
    struct Argx_Group *group;
} Argx_Value_Union;

typedef struct Argx {
    char c;
    So opt;
    So desc;
    Argx_Hint hint;
    Argx_Type_List id;
    Argx_Value_Union *val;  /* parsed value */
    Argx_Value_Union *ref;  /* reference / default value (refval) */
    VSo sources;            /* from where the value gets set -- should be an array if is_array, else, a single value. optional with a line number (some.config:123) */
    struct Argx_Group *group_p; /* always set to parent group */
    struct Argx_Group *group_s; /* only set if id == ARGX_GROUP */
    bool is_array;
    bool is_enum;
    bool is_env;
    int val_enum;
} Argx, *V_Argx;

LUT_INCLUDE(T_Argx, t_argx, So, BY_VAL, Argx, BY_VAL);

void v_argx_free(V_Argx *vargs);

typedef struct Argx_So {
    Argx *argx;
    So set_val;
    So set_ref;
    So hint;
    So hierarchy;
    bool val_visible;
    bool val_config;
    bool val_group;     /* if the set_val and set_ref are part of a group = require further expanding for a config */
    bool have_hint;
} Argx_So;

typedef struct Argx_Fmt {
} Argx_Fmt;

void argx_so_free(Argx_So *xso);
void argx_so_clear(Argx_So *xso);
void argx_so(Argx_So *xso, Argx_Fmt *fmt, Argx *argx);

void argx_fmt_help(So *out, Argx *argx);
void argx_fmt_config(So *out, Argx *argx);

#define RLARG_ARG_X_H
#endif /* RLARG_ARG_X_H */

