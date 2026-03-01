#ifndef RLARG_ARGX_GROUP_H

#include "argx.h"

typedef enum {
    ARGX_GROUP_ROOT,
    ARGX_GROUP_ENUM,
    ARGX_GROUP_OPTIONS,
    ARGX_GROUP_FLAGS,
} Argx_Group_List;

typedef struct Argx_Group {
    struct Arg *arg;
    V_Argx *list;
    T_Argx *table;
    So name;
    Argx_Group_List id;
    struct Argx *parent;
    bool config_print;
} Argx_Group, **Argx_Groups;

Argx_Group argx_group_init(struct Arg *arg, T_Argx *table, So name, Argx_Group_List id, Argx *parent);

void argx_group_free(Argx_Group *group);
void argx_groups_free(Argx_Groups group);

void argx_group_fmt_help(So *out, Argx_Group *group);
void argx_group_fmt_config(So *out, Argx_Group *group);

#define RLARG_ARGX_GROUP_H
#endif /* RLARG_ARGX_GROUP_H */

