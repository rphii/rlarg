#include "arg.h"

void argx_group_free(Argx_Group *group) {
    t_argx_free(group->table);
    v_argx_free(group->list);
}

struct Argx_Group *argx_group(struct Arg *arg, So name) {
    ASSERT_ARG(arg);
    /* check if the group already exists */
    for(Argx_Group *g = arg->groups; g < array_itE(arg->groups); ++g) {
        if(!so_cmp(g->name, name)) {
            return g;
        }
    }
    /* create new group */
    Argx_Group result = {
        .name = name,
        .table = &arg->table,
        .arg = arg,
    };
    array_push(arg->groups, result);
    return array_itL(arg->groups);
}

void argx_group_fmt_help(So *out, Argx_Group *group) {
    ASSERT_ARG(out);
    ASSERT_ARG(group);
    so_fmt(out, "%.*s:\n", SO_F(group->name));
    for(Argx **argx = group->list; argx < array_itE(group->list); ++argx) {
        argx_fmt_help(out, *argx);
    }
}

void argx_group_fmt_config(So *out, Argx_Group *group) {
    ASSERT_ARG(out);
    ASSERT_ARG(group);
    //so_fmt(out, "# %.*s\n", SO_F(group->name));
    for(Argx **argx = group->list; argx < array_itE(group->list); ++argx) {
        //so_fmt(out, "%.*s.", SO_F(group->name));
        argx_fmt_config(out, *argx);
    }
    so_push(out, '\n');
}

