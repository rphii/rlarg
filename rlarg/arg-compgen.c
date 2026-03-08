#include "arg-compgen.h"
#include "arg.h"
#include "argx-so.h"

#define ARG_COMPGEN_DELIM   0   //'\n'

void arg_compgen_group(Argx_Group *group) {
    if(!group) return;
    bool treat_as_options = (group->table == &group->arg->t_opt);
    Argx **itE = array_itE(group->list);
    for(Argx **it = group->list; it < itE; ++it) {
        printf("%c", ARG_COMPGEN_DELIM);
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
            printf("true%cfalse", ARG_COMPGEN_DELIM);
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
    ASSERT_ARG(arg);
    for(Argx_Group **group = arg->opts; group < array_itE(arg->opts); ++group) {
        arg_compgen_group(*group);
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
    if(arg->builtin.compgen_done) return;
    arg->builtin.compgen_done = true;
    static_arg_compgen_argx(arg, argx);
    printf("\n");
}

void arg_compgen_help_argx(struct Arg *arg, struct Argx *argx) {
    Argx_So xso = {0};
    if(argx->id == ARGX_TYPE_GROUP) {
        arg_compgen_help_group(arg, argx->group_s);
    } else {
        argx_so(&xso, argx, true, false);
        printf("%c%.*s%.*s", ARG_COMPGEN_DELIM, SO_F(xso.hierarchy), SO_F(argx->opt));
    }
    argx_so_free(&xso);
}

void arg_compgen_help_group(struct Arg *arg, struct Argx_Group *group) {
    Argx_So xso = {0};
    Argx **itE = array_itE(group->list);
    for(Argx **it = group->list; it < itE; ++it) {
        argx_so_clear(&xso);
        argx_so(&xso, *it, true, false);
        printf("%c%.*s%.*s", ARG_COMPGEN_DELIM, SO_F(xso.hierarchy), SO_F((*it)->opt));
        if((*it)->id == ARGX_TYPE_GROUP) {
            printf(".");
        }
    }

    argx_so_free(&xso);
}

void arg_compgen_help_groups(struct Arg *arg) {
    Argx_Group **itE = array_itE(arg->opts);
    for(Argx_Group **it = arg->opts; it < itE; ++it) {
        printf("%c%.*s.", ARG_COMPGEN_DELIM, SO_F((*it)->name));
    }
    printf("%c%.*s.", ARG_COMPGEN_DELIM, SO_F(arg->pos.name));
    printf("%c%.*s.", ARG_COMPGEN_DELIM, SO_F(arg->env.name));
}


