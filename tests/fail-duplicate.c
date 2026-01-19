#include "../rlarg.h"
#include <rlc.h>

int main(void) {
    struct Arg *arg = arg_new();
    ASSERT(arg, "expect to have pointer");

    struct Argx_Group *g1 = argx_group(arg, so("default"));

    argx(g1, 0, so("just-some-switch-here"), so("desc"));
    argx(g1, 0, so("just-some-switch-here"), so("desc"));

    return 0;
}

