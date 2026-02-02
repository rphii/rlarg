#include <rlc.h>
#include "arg.h"

void arg_free(struct Arg **arg) {
    if(!arg) return;
    array_free_ext((*arg)->opts, argx_group_free);
    argx_group_free(&(*arg)->pos);
    argx_group_free(&(*arg)->env);
    array_free((*arg)->queue);
    vso_free(&(*arg)->builtin.sources_paths);
    vso_free(&(*arg)->builtin.sources_content);
    free(*arg);
    *arg = 0;
}

struct Arg *arg_new(void) {
    Arg *result;
    NEW(Arg, result);
    result->pos = argx_group_init(result, &result->t_pos, so("positional"), ARGX_GROUP_ROOT, 0);
    result->env = argx_group_init(result, &result->t_env, so("environment"), ARGX_GROUP_ROOT, 0);

#if 0
        So_Fx program;
        So_Fx group;
        So_Fx group_delim;
        So_Fx pos;
        So_Fx c;
        So_Fx opt;
        So_Fx env;
        So_Fx desc;
        So_Fx enum_unset;
        So_Fx enum_set;
        So_Fx enum_delim;
        So_Fx flag_unset;
        So_Fx flag_set;
        So_Fx flag_delim;
        So_Fx hint;
        So_Fx hint_delim;
        So_Fx val;
        So_Fx val_delim;
#endif
    result->rice.c = (So_Fx){ .fg = COLOR_BLUE };
    result->rice.opt = (So_Fx){ .fg = COLOR_AQUA };
    result->rice.hint = (So_Fx){ .fg = COLOR_GREEN };
    result->rice.enum_set = (So_Fx){ .fg = COLOR_YELLOW, .bold = true, .underline = true };
    result->rice.enum_unset = (So_Fx){ .fg = COLOR_OLIVE };
    result->rice.enum_delim = (So_Fx){ .fg = COLOR_GRAY };
    result->rice.flag_set = (So_Fx){ .fg = COLOR_FUCHSIA, .bold = true, .underline = true };
    result->rice.flag_unset = (So_Fx){ .fg = COLOR_PURPLE };
    result->rice.flag_delim = (So_Fx){ .fg = COLOR_GRAY };
    result->rice.hint = (So_Fx){ .fg = COLOR_LIME };
    result->rice.hint_delim = (So_Fx){ .fg = COLOR_GRAY };

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

    argx_so(&xso, true, help);
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

void arg_enable_config_print(struct Arg *arg, bool enable) {
    arg->builtin.config_use_builtin = enable;
}

void arg_config(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    for(Argx_Group *group = arg->opts; group < array_itE(arg->opts); ++group) {
        if(!group->config_print) continue;
        argx_group_fmt_config(&out, group);
    }
    so_print(out);
    so_free(&out);
}

