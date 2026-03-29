#include "../rlarg.h"
#include <rlc.h>

int main(void) {
    struct Arg *arg = arg_new(0);
    ASSERT(arg, "expect to have pointer");

    struct Argx_Group *g1 = argx_group(arg, so("default"));

    argx_opt(g1, 0, so("just-some-switch-here"), so("desc"));
    argx_opt(g1, 0, so("just-some-switch-here"), so("desc"));

    return 0;
}

