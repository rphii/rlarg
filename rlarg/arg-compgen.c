#include "arg-compgen.h"
#include "arg.h"

void arg_compgen_group(Argx_Group *group, bool *any_out) {
    if(!group) return;
    ASSERT_ARG(any_out);
    bool treat_as_options = (group == group->arg->opts);
    Argx **itE = array_itE(group->list);
    for(Argx **it = group->list; it < itE; ++it) {
        if(treat_as_options) printf("--");
        printf("%.*s", SO_F((*it)->opt));
        if(it + 1 < itE) printf("%c", 0);
        *any_out = true;
    }
}

static void static_arg_compgen_argx(struct Arg *arg, struct Argx *argx, bool *any_out) {
    ASSERT_ARG(any_out);
    if(!argx) return;
    switch(argx->id) {
        default: ABORT(ERR_UNREACHABLE("unhandled id %u"), argx->id);
        case ARGX_TYPE_NONE:
        case ARGX_TYPE_INT:
        case ARGX_TYPE_SIZE:
        case ARGX_TYPE_COLOR:
        case ARGX_TYPE_STRING: break; /* can not provide compgen */
        case ARGX_TYPE_BOOL: {
            if(*any_out) printf("%c", 0);
            printf("true%cfalse", 0);
            *any_out = true;
        } break;
        case ARGX_TYPE_FLAG:
        case ARGX_TYPE_ENUM: {
            if(*any_out) printf("%c", 0);
            printf("%.*s", SO_F(argx->opt));
            *any_out = true;
        } break;
        case ARGX_TYPE_GROUP: {
            Argx_Group *sub = argx->group_s;
            ASSERT_ARG(sub);
            arg_compgen_group(sub, any_out);
        } break;
    }
}


void arg_compgen_global(struct Arg *arg) {
    if(arg->builtin.compgen_done) return;
    arg->builtin.compgen_done = true;
    printff("GLOBAL COMPGEN");
    bool any_out = false;
    ASSERT_ARG(arg);
    for(Argx_Group *group = arg->opts; group < array_itE(arg->opts); ++group) {
        arg_compgen_group(group, &any_out);
    }
    arg_compgen_group(&arg->env, &any_out);
    arg_compgen_group(&arg->pos, &any_out);
    /* also print the argx information about the next positional in line */
    Argx *argx = arg->i_pos < array_len(arg->pos.list) ? array_at(arg->pos.list, arg->i_pos) : 0;
    static_arg_compgen_argx(arg, argx, &any_out);
    printf("\n");
}

void arg_compgen_argx(struct Arg *arg, struct Argx *argx) {
    if(arg->builtin.compgen_flags) {
        arg_compgen_global(arg);
        return;
    }
    bool any_out = false;
    if(arg->builtin.compgen_done) return;
    arg->builtin.compgen_done = true;
    static_arg_compgen_argx(arg, argx, &any_out);
    printf("\n");
}


