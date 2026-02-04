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

/* TODO maybe not an attribute... cuz not in attr... */
void argx_attr_callback(struct Argx *argx, Argx_Function func, void *user, Argx_Priority_List priority) {
    ASSERT_ARG(argx);
    ASSERT_ARG(func);
    argx->callback.func = func;
    argx->callback.user = user;
    argx->callback.priority = priority;
}


