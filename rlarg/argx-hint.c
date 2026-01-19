#include "arg.h"

void argx_hint_kind(struct Argx *argx, Argx_Hint_List id) {
    ASSERT_ARG(argx);
    argx->hint.id = id;
}

void argx_hint_text(struct Argx *argx, So text) {
    ASSERT_ARG(argx);
}


