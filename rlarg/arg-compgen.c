#include "arg-compgen.h"
#include "arg.h"

void arg_compgen_group(Argx_Group *group) {
    if(!group) return;
    bool treat_as_options = (group == group->arg->opts);
    Argx **itE = array_itE(group->list);
    for(Argx **it = group->list; it < itE; ++it) {
        printf("%c", 0);
        if(treat_as_options) printf("--");
        printf("%.*s", SO_F((*it)->opt));
    }
}

static void static_arg_compgen_argx(struct Arg *arg, struct Argx *argx) {
    if(!argx) return;
    switch(argx->id) {
        default: ABORT(ERR_UNREACHABLE("unhandled id %u"), argx->id);
        case ARGX_TYPE_NONE:
        case ARGX_TYPE_INT:
        case ARGX_TYPE_SIZE:
        case ARGX_TYPE_COLOR:
        case ARGX_TYPE_STRING: break; /* can not provide compgen */
        case ARGX_TYPE_BOOL: {
            printf("%c", 0);
            printf("true%cfalse", 0);
        } break;
        case ARGX_TYPE_FLAG:
        case ARGX_TYPE_ENUM: {
            printf("%c", 0);
            printf("%.*s", SO_F(argx->opt));
        } break;
        case ARGX_TYPE_GROUP: {
            Argx_Group *sub = argx->group_s;
            ASSERT_ARG(sub);
            arg_compgen_group(sub);
        } break;
    }
}


void arg_compgen_global(struct Arg *arg) {
    if(arg->builtin.compgen_done) return;
    arg->builtin.compgen_done = true;
    //printff("GLOBAL COMPGEN");
    bool any_out = true;
    ASSERT_ARG(arg);
    for(Argx_Group *group = arg->opts; group < array_itE(arg->opts); ++group) {
        arg_compgen_group(group);
    }
    arg_compgen_group(&arg->env);
    arg_compgen_group(&arg->pos);
    /* also print the argx information about the next positional in line BUT only if the current help is not the --help argx */
    if(arg->help.last != arg->help.argx) {
        Argx *argx = arg->i_pos < array_len(arg->pos.list) ? array_at(arg->pos.list, arg->i_pos) : 0;
        static_arg_compgen_argx(arg, argx);
    }
    printf("\n");
}

void arg_compgen_argx(struct Arg *arg, struct Argx *argx) {
    if(arg->builtin.compgen_flags) {
        arg_compgen_global(arg);
        return;
    }
    bool any_out = true;
    if(arg->builtin.compgen_done) return;
    arg->builtin.compgen_done = true;
    static_arg_compgen_argx(arg, argx);
    printf("\n");
}


