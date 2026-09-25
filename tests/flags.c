#include "../rlarg.h"
#include <rlc.h>

typedef struct {
    bool f1, f2, f3;
} Flags;

int flag_callback(struct Argx *x, void *p, So s) {

    Flags *f = p;

    printf(" CB :: %.*s ( %u,%u,%u )\n", SO_F(s), f->f1, f->f2, f->f3);

    return 0;
}

int main(int argc, const char **argv) {

    int err = -1;
    bool quit_early = false;

    struct Arg_Config *cfg = arg_config_new();
    arg_config_set_program(cfg, so_l(argv[0]));
    struct Arg *arg = arg_new(cfg);

    struct Argx_Group *g;
    struct Argx *x, *xs;

    Flags f = {0};

    argx_builtin_rice(arg);
    g=argx_group(arg, so("default"));

    argx_builtin_opt_help(g, ARGX_BUILTIN_OPT_HELP);

    xs=argx_opt(g, 'F', so("flags-all"), so("set all flags"));
       argx_type_switch(xs);
       argx_callback(xs, flag_callback, &f, ARGX_PRIORITY_IMMEDIATELY);

    x=argx_opt(g, 'f', so("flags"), so("some flags"));
      g=argx_group_flags(x);
        x=argx_flag(g, &f.f1, 0, so("f1"), so("flag 1"));
          argx_callback(x, flag_callback, &f, ARGX_PRIORITY_IMMEDIATELY);
          argx_switch_flag(xs, x, true);
        x=argx_flag(g, &f.f2, 0, so("f2"), so("flag 2"));
          argx_callback(x, flag_callback, &f, ARGX_PRIORITY_IMMEDIATELY);
          argx_switch_flag(xs, x, true);
        x=argx_flag(g, &f.f3, 0, so("f3"), so("flag 3"));
          argx_callback(x, flag_callback, &f, ARGX_PRIORITY_IMMEDIATELY);
          argx_switch_flag(xs, x, true);

    if((err = arg_parse(arg, argc, argv, &quit_early))) goto defer;
    if(quit_early) goto defer;

defer:
    return err;
}

