#ifndef RLARG_ARGX_GROUP_H

#include "argx.h"

typedef struct Argx_Group {
    struct Arg *arg;
    V_Argx *list;
    T_Argx *table;
    So name;
} Argx_Group, *Argx_Groups;

void argx_group_fmt_help(So *out, Argx_Group *group);

#define RLARG_ARGX_GROUP_H
#endif /* RLARG_ARGX_GROUP_H */

