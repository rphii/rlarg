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

void arg_runtime_quit_when_all_parsed(struct Argx *argx, bool val) {
    ASSERT_ARG(argx);
    if(!val) return;
    Argx_Group *group = argx->group_p;
    ASSERT_ARG(group);
    Arg *arg = group->arg;
    arg->builtin.quit_when_all_parsed = val;
}

void arg_runtime_set_parse_error_message(struct Argx *argx, So errmsg) {
    ASSERT_ARG(argx);
    Argx_Group *group = argx->group_p;
    ASSERT_ARG(group);
    Arg *arg = group->arg;
    arg->builtin.custom_err_msg = errmsg;
}

