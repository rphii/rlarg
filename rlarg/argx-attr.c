#include "argx-attr.h"
#include "argx.h"
#include "arg.h"

void argx_attr_hide(struct Argx *argx, bool hide) {
    ASSERT_ARG(argx);
    argx->attr.is_hidden = hide;
}

void argx_attr_configurable(struct Argx *argx, bool configurable) {
    ASSERT_ARG(argx);
    argx->attr.is_unconfigurable = !configurable;
}

void argx_attr_callback_skip_compgen(struct Argx *argx, bool skip) {
    ASSERT_ARG(argx);
    argx->attr.callback_skip_compgen = skip;
}

void argx_attr_explicit_bool(struct Argx *argx, bool explicit) {
    ASSERT_ARG(argx);
    ASSERT(argx->id == ARGX_TYPE_BOOL, "expect bool argx");
    argx->attr.is_explicit_bool = explicit;
}

void argx_attr_fatal_config_error(struct Argx *argx, bool fatal) {
    ASSERT_ARG(argx);
    argx->attr.is_fatal_config_error = fatal;
}

bool argx_attr_is_explicit_bool(struct Argx *argx) {
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(argx->group_p->arg);
    bool explicit_bool = argx->attr.is_explicit_bool || argx_is_subgroup_of_root(argx, &argx->group_p->arg->pos);
    return explicit_bool ;
}


