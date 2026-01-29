#include "../rlarg.h"

int main(const int argc, const char **argv) {
    struct Arg *arg = arg_new();
    ASSERT(arg, "expect to have pointer");

    bool val = 0;
    struct Argx_Group *g1 = argx_group(arg, so("default"));
    struct Argx *x=argx_opt(g1, 'b', so("bool"), so("a boolean value"));
    argx_type_bool(x, &val, 0);

    arg_parse(arg, argc, argv);

    arg_free(&arg);
    return 0;
}

