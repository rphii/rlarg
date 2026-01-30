#include "../rlarg.h"
#include "arg.h"

void arg_runtime_quit_early(struct Argx *argx, bool val) {
    ASSERT_ARG(argx);
    if(!val) return;
    Argx_Group *group = argx->group_p;
    ASSERT_ARG(group);
    Arg *arg = group->arg;
    arg->builtin.quit_early = val;
}

void arg_runtime_quit_when_all_valid(struct Argx *argx, bool val) {
    ASSERT_ARG(argx);
    if(!val) return;
    Argx_Group *group = argx->group_p;
    ASSERT_ARG(group);
    Arg *arg = group->arg;
    arg->builtin.quit_when_all_valid = val;
}

