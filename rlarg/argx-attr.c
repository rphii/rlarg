#include "argx-attr.h"
#include "argx.h"

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

