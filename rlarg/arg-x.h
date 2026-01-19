#ifndef RLARG_ARG_X_H

#include <rlso.h>
#include <rlc.h>
#include "argx-hint.h"

#define ARG_X_SHORT_COUNT   ('~' - '!')

typedef enum {
    ARGX_NONE,
    ARGX_INT,
    ARGX_SIZE,
    ARGX_BOOL,
    ARGX_STRING,
    ARGX_ENUM,
} Argx_List;

typedef enum {
    ARGX_PRIORITY_WHEN_VALID,
    ARGX_PRIORITY_IMMEDIATELY,
} Argx_Priority_List;

typedef union Argx_Value_Union {
    int i, e;
    ssize_t z;
    bool b;
    So so;
} Argx_Value_Union;

typedef struct Argx {
    char c;
    So opt;
    So desc;
    Argx_Hint hint;
    Argx_List id;
    Argx_Value_Union *val; /* parsed value */
    Argx_Value_Union *ref; /* reference / default value */
    struct Argx_Group *group;
} Argx, *V_Argx;

LUT_INCLUDE(T_Argx, t_argx, So, BY_VAL, Argx, BY_VAL);

typedef struct Argx_Group {
    struct Arg *arg;
    V_Argx *list;
    T_Argx *table;
    So name;
} Argx_Group, *Argx_Groups;

void v_argx_free(V_Argx *vargs);

typedef struct Argx_So {
    Argx *argx;
    So val;
    So ref;
    So hint;
    bool ref_visible;
    bool have_hint;
} Argx_So;

void argx_so_free(Argx_So *xso);
void argx_so_clear(Argx_So *xso);
void argx_so(Argx_So *xso, Argx *argx);
void argx_fmt(So *out, Argx *argx);

void argx_group_fmt(So *out, Argx_Group *group);

#define RLARG_ARG_X_H
#endif /* RLARG_ARG_X_H */

