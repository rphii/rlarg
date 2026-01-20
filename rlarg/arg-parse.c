#include "arg.h"

int arg_parse_stream(struct Arg *arg, const int argc, const char **argv) {
    /* set the stream */
    arg->stream.argc = argc;
    arg->stream.argv = argv;
    arg->stream.i = 0;
    /* now parse */
    return 0;
}

// TODO: we can't just OVERWRITE the argx->val ! with argx->ref ! because .. well they point to REAL locations.

void arg_parse_setref_group(Argx_Group *group) {
    if(!group) return;
    for(Argx **it = group->list; it < array_itE(group->list); ++it) {
        printff("setref for: %.*s",SO_F((*it)->opt));
        ASSERT_ARG(it);
        Argx *argx = *it;
        ASSERT_ARG(argx);
        if(argx->id == ARGX_GROUP) {
            if(!argx->group_s) continue;
            switch(argx->group_s->id) {
                case ARGX_GROUP_OPTIONS:
                case ARGX_GROUP_ROOT: arg_parse_setref_group(argx->group_s); break;
                case ARGX_GROUP_ENUM: argx->val = argx->ref; break;
                case ARGX_GROUP_FLAGS: ABORT("not yet implemented"); break;
            }
        } else {
            printff(" v %p = r %p",argx->val,argx->ref);
            argx->val = argx->ref;
        }
    }
}

void arg_parse_setref(struct Arg *arg) {
    /* apply values from references */
    for(Argx_Group *group = arg->groups; group < array_itE(arg->groups); ++group) {
        arg_parse_setref_group(group);
    }
}

int arg_parse(struct Arg *arg, const int argc, const char **argv) {

    arg_parse_stream(arg, argc, argv);

    arg_parse_setref(arg);

    return 0;
}

