#include <rlc.h>
#include "arg.h"

void arg_free(struct Arg **arg) {
    if(!arg) return;
    array_free_ext((*arg)->opts, argx_group_free);
    argx_group_free(&(*arg)->pos);
    argx_group_free(&(*arg)->env);
    array_free((*arg)->queue);
    free(*arg);
    *arg = 0;
}

struct Arg *arg_new(void) {
    Arg *result;
    NEW(Arg, result);
    result->pos = argx_group_init(result, &result->t_pos, so("positional"), ARGX_GROUP_ROOT, 0);
    result->env = argx_group_init(result, &result->t_env, so("environment"), ARGX_GROUP_ROOT, 0);
    return result;
}

void arg_help(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    argx_group_fmt_help(&out, &arg->pos);
    for(Argx_Group *group = arg->opts; group < array_itE(arg->opts); ++group) {
        argx_group_fmt_help(&out, group);
    }
    argx_group_fmt_help(&out, &arg->env);
    so_print(out);
    so_free(&out);
}

void arg_help_argx_rec(So *out, Argx *argx) {
    if(!argx) return;
    arg_help_argx_rec(out, argx->group_p ? argx->group_p->parent : 0);
    argx_fmt_help(out, argx);
}


void arg_help_argx(struct Argx *help) {
    So out = SO;
    Argx_So xso = {0};
    Argx_Fmt fmt = {};

    argx_so(&xso, &fmt, help);
    so_fmt(&out, "%.*s%.*s:\n", SO_F(xso.hierarchy), SO_F(xso.argx->opt));
    argx_so_free(&xso);

    arg_help_argx_rec(&out, help);

    size_t len = array_len(help->sources);
    if(len) {
        so_fmt(&out, "sources:\n");
        for(size_t i = 0; i < len; ++i) {
            Arg_Stream_Source src = array_at(help->sources, i);
            if(src.line_number) {
                so_fmt(&out, "  %.*s:%u%s\n", SO_F(src.path), src.line_number, i + 1 < len ? "," : "");
            } else {
                so_fmt(&out, "  %.*s%s\n", SO_F(src.path), i + 1 < len ? "," : "");
            }
        }
    }

    so_print(out);
    so_free(&out);
}

void arg_config(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    for(Argx_Group *group = arg->opts; group < array_itE(arg->opts); ++group) {
        argx_group_fmt_config(&out, group);
    }
    so_print(out);
    so_free(&out);
}

