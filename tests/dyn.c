#include "../rlarg.h"

typedef struct {
    VSo subs;
    VSo *icons;
    struct Argx_Group *dyn;
} Dyn;

void icon_free(So **so) {
    free(*so);
}

int cb(struct Argx *argx, void *user, So so) {

    Dyn *dyn = user;

    So *it;
    NEW(So, it);
    array_push(dyn->icons, it);
    So sub = array_atL(dyn->subs);
    struct Argx *x = argx_opt(dyn->dyn, 0, sub, SO);
    argx_type_so(x, it, 0);

    return 0;
}

int main(int argc, const char **argv) {

    int result = -1;
    bool quit_early = false;

    Dyn dyn = {0};
    struct Arg *arg = arg_new();

    struct Argx_Group *g = argx_group(arg, so("default"));
    struct Argx *x;

    arg_enable_config_print(arg, true);
    argx_builtin_opt_help(g);
    argx_builtin_opt_source(g, so("dyn.conf"));

    x=argx_opt(g, 0, so("sub"), so("add things"));
      argx_type_array_so(x, &dyn.subs, 0);
      argx_callback(x, cb, &dyn, ARGX_PRIORITY_IMMEDIATELY);
    x=argx_opt(g, 0, so("icons"), so("specify icons"));
      dyn.dyn=argx_group_options(x);

    g=argx_builtin_rice(arg);
    argx_builtin_opt_color(g);

    if(arg_parse(arg, argc, argv, &quit_early)) goto defer;
    result = 0;
    if(quit_early) goto defer;

defer:

    array_free_ext(dyn.icons, icon_free);
    arg_free(&arg);

    return result;
}

