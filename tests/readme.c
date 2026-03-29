#include "../rlarg.h"

typedef struct Readme {
    int i;
    int *vi;
} Readme;

int main(int argc, const char **argv) {
    Readme r = {0};

    /* set up arg */
    struct Arg *arg = arg_new(0);
    struct Argx_Group *g = argx_group(arg, so("default"));
    struct Argx *x;
    argx_builtin_opt_color(g);
    argx_builtin_opt_help(g);
    arg_enable_config_print(arg, true);
    argx_builtin_opt_source(g, so("readme.conf"));

    /* add options */
    x=argx_opt(g, 0, so("int"), so("an integer"));
      argx_type_int(x, &r.i, 0);
    x=argx_opt(g, 0, so("vint"), so("an integer array"));
      argx_type_array_int(x, &r.vi, 0);

    /* parse */
    bool quit_early = false;
    int result = 0;
    result = arg_parse(arg, argc, argv, &quit_early);
    if(result || quit_early) goto defer;

    /* main code */
    printf("Hello, README! The int is: %i\n", r.i);

defer:
    arg_free(&arg);
    return result;
}

