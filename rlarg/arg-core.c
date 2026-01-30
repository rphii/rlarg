#include <rlc.h>
#include "arg.h"

void arg_free(struct Arg **arg) {
    if(!arg) return;
    array_free_ext((*arg)->opts, argx_group_free);
    argx_group_free(&(*arg)->pos);
    argx_group_free(&(*arg)->env);
    array_free((*arg)->queue);
    free(*arg);
    *arg = 0;
}

struct Arg *arg_new(void) {
    Arg *result;
    NEW(Arg, result);
    result->pos = argx_group_init(result, &result->t_pos, so("positional"), ARGX_GROUP_ROOT, 0);
    result->env = argx_group_init(result, &result->t_env, so("environment"), ARGX_GROUP_ROOT, 0);
    return result;
}

void arg_help(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    argx_group_fmt_help(&out, &arg->pos);
    for(Argx_Group *group = arg->opts; group < array_itE(arg->opts); ++group) {
        argx_group_fmt_help(&out, group);
    }
    argx_group_fmt_help(&out, &arg->env);
    so_print(out);
    so_free(&out);
}

void arg_config(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    for(Argx_Group *group = arg->opts; group < array_itE(arg->opts); ++group) {
        argx_group_fmt_config(&out, group);
    }
    so_print(out);
    so_free(&out);
}

