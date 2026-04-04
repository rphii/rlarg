#include "../rlarg.h"
#include "arg.h"

void arg_runtime_quit_early(struct Argx *argx, bool val) {
    if(!argx) return;
    if(!val) return;
    Argx_Group *group = argx->group_p;
    ASSERT_ARG(group);
    Arg *arg = group->arg;
    arg->builtin.quit_early = val;
}

void arg_runtime_quit_when_all_parsed(struct Argx *argx, bool val) {
    if(!argx) return;
    if(!val) return;
    Argx_Group *group = argx->group_p;
    ASSERT_ARG(group);
    Arg *arg = group->arg;
    arg->builtin.quit_when_all_parsed = val;
}

void arg_runtime_set_parse_error_message(struct Argx *argx, char *fmt, ...) {
    if(!argx) return;
    Argx_Group *group = argx->group_p;
    ASSERT_ARG(group);
    Arg *arg = group->arg;
    so_clear(&arg->builtin.custom_err_msg);
    va_list va;
    va_start(va, fmt);
    so_fmt_va(&arg->builtin.custom_err_msg, fmt, va);
    va_end(va);
}

