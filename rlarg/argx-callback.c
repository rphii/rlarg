#include "argx.h"
#include "argx-callback.h"

void argx_callback(struct Argx *argx, Argx_Function func, void *user, Argx_Priority_List priority) {
    ASSERT_ARG(argx);
    ASSERT_ARG(func);
    argx->callback.func = func;
    argx->callback.user = user;
    argx->callback.priority = priority;
}

