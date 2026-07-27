#include "../rlarg.h"
#include <rlc.h>

int main(int argc, const char **argv) {

    int err = -1;
    bool quit_early = false;

    struct Arg_Config *cfg = arg_config_new();
    arg_config_set_program(cfg, so_l(argv[0]));
    struct Arg *arg = arg_new(cfg);

    struct Argx_Group *h, *s, *g = argx_group(arg, so("default"));
    struct Argx *x, *y, *z;

    bool f1 = false, f2 = false;
    int action = 0;
    So input = SO, output = SO;
    bool do_thing = false;

    argx_builtin_rice(arg);
    argx_builtin_opt_help(g, ARGX_BUILTIN_OPT_HELP);

    x=argx_pos(arg, so("thing"), so("thing to do"));
    //x=argx_opt(g, 0, so("thing"), so("thing to do"));
      g=argx_group_enum(x, &action, 0);
        x=argx_enum_bind(g, 1, so("1"), so("one"));
          s=argx_group_sequence(x);
            y=argx_opt(s, 0, so("input"), so("input file"));
              //argx_type_so(y, &input, 0);
              h=argx_group_sequence(y);
                z=argx_opt(h, 0, so("hi"), so("asdf"));
                z=argx_opt(h, 0, so("ih"), so("fdsa"));
            y=argx_opt(s, 0, so("output"), so("output file"));
              argx_type_so(y, &input, 0);
            y=argx_opt(s, 0, so("flag"), so("flags"));
              h=argx_group_flags(y);
                argx_flag(h, &f1, 0, so("f1"), so("flag 1"));
                argx_flag(h, &f2, 0, so("f2"), so("flag 2"));
            y=argx_opt(s, 0, so("bool"), so("a boolean"));
              argx_type_bool(y, &do_thing, 0);


    if((err = arg_parse(arg, argc, argv, &quit_early))) goto defer;
    if(quit_early) goto defer;

    printf("Action: %u\n", action);

defer:
    return err;
}

