#ifndef RLARG_ARG_X_H

#include <rlso.h>
#include <rlc.h>
#include "../rlarg.h"
#include "argx-hint.h"
#include "argx-so.h"

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
} Argx_Value_Union;

typedef struct Argx_Callback {
    Argx_Priority_List priority;
    Argx_Function func;
    void *user;
} Argx_Callback;

#define ARGX_SOURCE_REFVAL  so("refval")
#define ARGX_SOURCE_STDIN   so("stdin")
#define ARGX_SOURCE_ENVVARS so("envvars")


typedef struct Argx_Callback_Queue {
    struct Argx *argx;
    So so;
} Argx_Callback_Queue;

typedef struct Argx {
    Argx_Value_Union val;  /* parsed value */
    Argx_Value_Union ref;  /* reference / default value (refval) */
    Argx_Hint hint;
    Argx_Type_List id;
    Arg_Stream_Source *sources;  /* from where the value gets set -- should be an array if is_array, else, a single value. optional with a line number (some.config:123) */
    struct Argx_Group *group_p; /* always set to parent group */
    struct Argx_Group *group_s; /* only set if id == ARGX_GROUP */
    Argx_Callback callback;
    bool is_array;
    bool is_env;
    bool is_hidden;
    bool is_unconfigurable;
    int val_enum;
    So desc;
    So opt;
    char c;
} Argx, *V_Argx;

LUT_INCLUDE(T_Argx, t_argx, So, BY_VAL, Argx, BY_VAL);

void v_argx_free(V_Argx *vargs);

void argx_fmt_help(So *out, Argx *argx);
void argx_fmt_config(So *out, Argx *argx);

bool argx_flag_is_any_source_set(Argx *argx);
bool argx_is_configurable(Argx *argx);

void argx_builtin_env_config(struct Arg *arg);

#define RLARG_ARG_X_H
#endif /* RLARG_ARG_X_H */

