#include "../rlarg.h"

int main(const int argc, const char **argv) {
    struct Arg *arg = arg_new(0);
    int status = 0;
    ASSERT(arg, "expect to have pointer");

    bool val = 0;
    struct Argx_Group *g1 = argx_group(arg, so("default"));
    struct Argx *x=argx_opt(g1, 'b', so("bool"), so("a boolean value"));
    argx_type_bool(x, &val, 0);
    argx_builtin_opt_color(g1);
    //argx_builtin_opt_help(g1);
    argx_builtin_env_compgen(arg);
    arg_enable_config_print(arg, true);

    bool quit_early = false;
    if(arg_parse(arg, argc, argv, &quit_early)) goto defer;
    if(quit_early) goto defer;

defer:
    arg_free(&arg);
    return status;
}

