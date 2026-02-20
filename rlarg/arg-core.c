#include <rlc.h>
#include "arg.h"

void arg_free(struct Arg **arg) {
    if(!arg) return;
    array_free_ext((*arg)->opts, argx_groups_free);
    argx_group_free(&(*arg)->pos);
    argx_group_free(&(*arg)->env);
    array_free((*arg)->queue);
    vso_free(&(*arg)->builtin.sources_paths);
    vso_free(&(*arg)->builtin.sources_content);
    vso_free(&(*arg)->help.sub);
    free(*arg);
    *arg = 0;
}

struct Arg *arg_new(void) {
    Arg *result;
    NEW(Arg, result);
    result->pos = argx_group_init(result, &result->t_pos, so("positional"), ARGX_GROUP_ROOT, 0);
    result->env = argx_group_init(result, &result->t_env, so("environment"), ARGX_GROUP_ROOT, 0);

    bool *nofx = &result->builtin.nocolor;

    result->rice.program =      (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0xdd, 0x55, 0x55), .bold = true };
    result->rice.group =        (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0xcc, 0xcc, 0xcc), .bold = true, .underline = true };
    result->rice.group_delim =  (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x66, 0x66, 0x66) };
    result->rice.pos =          (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0xaa, 0x55, 0xdd), .italic = true };
    result->rice.c =            (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0xb9, 0x7b, 0x97) };
    result->rice.opt =          (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0xb9, 0x7b, 0x97), .bold = true };
    result->rice.env =          (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x01, 0x01, 0x01), .bold = true, .bg = COLOR_RGB(0x86, 0x58, 0x65) };
    result->rice.desc =         (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0xaa, 0xaa, 0xaa) };
    result->rice.subopt =       (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x33, 0x66, 0xee) };
    result->rice.subopt_delim = (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x66, 0x66, 0x66) };
    result->rice.enum_unset =   (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x55, 0x55, 0xbb) };
    result->rice.enum_set =     (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x77, 0x77, 0xdd), .bold = true, .underline = true };
    result->rice.enum_delim =   (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x66, 0x66, 0x66) };
    result->rice.flag_unset =   (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0xbb, 0xbb, 0x55) };
    result->rice.flag_set =     (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0xdd, 0xdd, 0x77), .bold = true, .underline = true };
    result->rice.flag_delim =   (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x66, 0x66, 0x66) };
    result->rice.hint =         (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x55, 0xaa, 0xdd) };
    result->rice.hint_delim =   (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x66, 0x66, 0x66) };
    result->rice.val =          (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x55, 0xdd, 0x55) };
    result->rice.val_delim =    (So_Fx){ .nocolor = nofx, .fg = COLOR_RGB(0x66, 0x66, 0x66) };

    return result;
}

void arg_help(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    argx_group_fmt_help(&out, &arg->pos);
    for(Argx_Group **group = arg->opts; group < array_itE(arg->opts); ++group) {
        argx_group_fmt_help(&out, *group);
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

void arg_help_argx_group(struct Argx_Group *group) {
    ASSERT_ARG(group);
    So out = SO;
    argx_group_fmt_help(&out, group);
    so_print(out);
    so_free(&out);
}

void arg_help_argx(struct Argx *help) {
    So out = SO;
    Argx_So xso = {0};
    ASSERT_ARG(help->group_p);
    ASSERT_ARG(help->group_p->arg);
    Arg_Rice *rice = &help->group_p->arg->rice;

    argx_so(&xso, help, false, false);
    so_fmt(&out, "%.*s", SO_F(xso.hierarchy));
    so_fmt_fx(&out, rice->group, 0, "%.*s", SO_F(xso.argx->opt));
    so_fmt_fx(&out, rice->group_delim, 0, ":");
    so_push(&out, '\n');
    argx_so_free(&xso);

    arg_help_argx_rec(&out, help);
    if(help->id == ARGX_TYPE_GROUP) {
        so_push(&out, '\n');
        ASSERT_ARG(help->group_s);
        Argx **itE = array_itE(help->group_s->list);
        for(Argx **it = help->group_s->list; it < itE; ++it) {
            argx_fmt_help(&out, *it);
        }
    }

    size_t len = array_len(help->sources);
    if(len) {
        so_fmt(&out, "\nsources:\n");
        for(size_t i = 0; i < len; ++i) {
            Arg_Stream_Source src = array_at(help->sources, i);
            if(src.line_number) {
                so_fmt(&out, "  %.*s:%u%s\n", SO_F(src.path), src.line_number, i + 1 < len ? "," : "");
            } else {
                so_fmt(&out, "  %.*s%s\n", SO_F(src.path), i + 1 < len ? "," : "");
            }
        }
    } else {
        so_fmt(&out, "\nnot set anywhere\n");
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
    for(Argx_Group **group = arg->opts; group < array_itE(arg->opts); ++group) {
        if(!(*group)->config_print) continue;
        argx_group_fmt_config(&out, *group);
    }
    so_print(out);
    so_free(&out);
}

