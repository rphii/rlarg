#include <rlc.h>
#include "arg.h"

void arg_free(struct Arg **arg) {
    if(!arg) return;
    for(Argx_Group *group = (*arg)->groups; group < array_itE((*arg)->groups); ++group) {
        argx_group_free(group);
    }
    *arg = 0;
}

struct Arg *arg_new(void) {
    Arg *result;
    NEW(Arg, result);
    return result;
}

void arg_help(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    for(Argx_Group *group = arg->groups; group < array_itE(arg->groups); ++group) {
        argx_group_fmt_help(&out, group);
    }
    so_print(out);
    so_free(&out);
}

void arg_config(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    for(Argx_Group *group = arg->groups; group < array_itE(arg->groups); ++group) {
        argx_group_fmt_config(&out, group);
    }
    so_print(out);
    so_free(&out);
}

