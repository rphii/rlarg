#include <rlc.h>
#include "arg.h"

struct Arg *arg_new(void) {
    Arg *result;
    NEW(Arg, result);
    return result;
}

void arg_help(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    for(Argx_Group *group = arg->groups; group < array_itE(arg->groups); ++group) {
        argx_group_fmt(&out, group);
    }
    so_print(out);
    so_free(&out);
}

