#include "arg.h"

void argx_hint_text(struct Argx *argx, So text) {
    ASSERT_ARG(argx);
    argx->hint.so = text;
}


