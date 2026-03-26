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

void arg_init_al(struct Arg *arg) {
    arg->print.bounds.c = 2;
    arg->print.bounds.opt = 8;
    arg->print.bounds.desc = 50;
    arg->print.bounds.max = 80;

    const size_t bc = arg->print.bounds.c;
    const size_t bo = arg->print.bounds.opt;
    const size_t bd = arg->print.bounds.desc;
    const size_t bm = arg->print.bounds.max;

    so_al_config(&arg->rice.group.align,          0,    bc,   bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.group_delim.align,    0,    bc,   bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.hint.align,           bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.hint_delim.align,     bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.enum_set.align,       bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.enum_delim.align,     bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.enum_unset.align,     bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.flag_set.align,       bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.flag_delim.align,     bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.flag_unset.align,     bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.sw.align,             bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.sw_delim.align,       bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.sequence.align,       bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.sequence_delim.align, bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.subopt.align,         bo,  bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.subopt_delim.align,   bo,   bo+4, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.val.align,            bo+8, bo+8, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.val_delim.align,      bo+8, bo+8, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.c.align,              bc,   bc,   bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.opt.align,            bo,   bo+2, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.env.align,            bc,   bc+2, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.desc.align,           bd,   bo+6, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.pos.align,            bc,   bc+2, bm, 0, &arg->print.p_al2);
    so_al_config(&arg->rice.program.align,        0,    0,    bm, 0, &arg->print.p_al2);
    so_al_config(&arg->print.whitespace,          0,    0,    bm, 0, &arg->print.p_al2);
    if(bm - bd >= 80 - bo) {
        arg->rice.val_delim.align.i0 = bd;
        arg->rice.val_delim.align.iNL = bd;
        arg->rice.val.align.i0 = bd + 2;
        arg->rice.val.align.iNL = bd + 2;
        arg->rice.desc.align.i0 = bd;
        arg->rice.desc.align.iNL = bd;
    }
}


struct Arg *arg_new(void) {
    Arg *result;
    NEW(Arg, result);
    result->pos = argx_group_init(result, &result->t_pos, so("positional"), ARGX_GROUP_ROOT, 0);
    result->env = argx_group_init(result, &result->t_env, so("environment"), ARGX_GROUP_ROOT, 0);

    arg_init_al(result);

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

void arg_help_short(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    /* format long options */
    so_fmt(&out, "Options: ");
    for(Argx_Group **group = arg->opts; group < array_itE(arg->opts); ++group) {
        Argx **xE = array_itE((*group)->list);
        for(Argx **xI = (*group)->list; xI < xE; ++xI) {
            so_fmt_fx(&out, arg->rice.opt, 0, "--%.*s", SO_F((*xI)->opt));
            so_fmt(&out, " ");
        }
    }
    
    /* format short options, TODO: if no short opts, it will still print the - without anything.. */
    so_fmt(&out, "\n\nShort options: ");
    so_fmt_fx(&out, arg->rice.opt, 0, "-");
    for(Argx_Group **group = arg->opts; group < array_itE(arg->opts); ++group) {
        Argx **xE = array_itE((*group)->list);
        for(Argx **xI = (*group)->list; xI < xE; ++xI) {
            if((*xI)->c) {
                so_fmt_fx(&out, arg->rice.opt, 0, "%c", (*xI)->c);
            }
        }
    }

    so_println(out);
    so_free(&out);
}

void arg_help_argx_rec(So *out, Argx *argx, bool full_help) {
    if(!argx) return;
    arg_help_argx_rec(out, argx->group_p ? argx->group_p->parent : 0, false);
    argx_fmt_help(out, argx, full_help);
}

void arg_help_argx_group(struct Argx_Group *group) {
    ASSERT_ARG(group);
    So out = SO;
    argx_group_fmt_help(&out, group);
    so_print(out);
    so_free(&out);
}

void argx_extend_sources(VSo *srces, Argx *argx) {

    ASSERT_ARG(argx);
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(argx->group_p->arg);

    size_t len = array_len(argx->sources);
    So tmp = SO;
    So hierarchy = SO;
    Arg_Rice *rice = &argx->group_p->arg->rice;
    if(len) {
        for(size_t i = 0; i < len; ++i) {
            so_zero(&tmp);
            Arg_Stream_Source src = array_at(argx->sources, i);
            switch(src.id) {
                case ARG_STREAM_SOURCE_CONFIG:
                    so_fmt(&tmp, "%.*s:%u", SO_F(src.path), src.number);
                    break;
                case ARG_STREAM_SOURCE_STDIN:
                    so_fmt(&tmp, "stdin@%u", src.number);
                    break;
                case ARG_STREAM_SOURCE_ENVVARS:
                    so_fmt(&tmp, "envvars");
                    break;
                case ARG_STREAM_SOURCE_REFVAL:
                    so_fmt(&tmp, "refval");
                    break;
                default: break;
            }
            if(so_len(tmp)) {
                so_clear(&hierarchy);
                argx_so_hierarchy(&hierarchy, rice, argx->group_p);
                so_fmt(&tmp, " :: %.*s%.*s", SO_F(hierarchy), SO_F(argx->opt));
                vso_push(srces, tmp);
            }
        }
    }
    so_free(&hierarchy);
}

void arg_help_argx(struct Argx *help) {
    So out = SO;
    VSo sources = 0;
    Argx_So xso = {0};
    Argx_So_Options opts = {0};
    ASSERT_ARG(help->group_p);
    ASSERT_ARG(help->group_p->arg);
    Arg_Rice *rice = &help->group_p->arg->rice;
    So_Align al_ws = help->group_p->arg->print.whitespace;
    bool full_help = true;

    argx_so(&xso, help, &opts);
    so_fmt_al(&out, rice->group.align, 0, "%.*s", SO_F(xso.hierarchy));
    so_fmt_fx(&out, rice->group, 0, "%.*s", SO_F(xso.argx->opt));
    so_fmt_fx(&out, rice->group_delim, 0, ":");
    so_al_nl(&out, al_ws, 1);
    argx_so_free(&xso);

    arg_help_argx_rec(&out, help, full_help);
    if(help->id == ARGX_TYPE_GROUP) {
        so_al_nl(&out, al_ws, 1);
        ASSERT_ARG(help->group_s);
        Argx **itE = array_itE(help->group_s->list);
        for(Argx **it = help->group_s->list; it < itE; ++it) {
            argx_fmt_help(&out, *it, true);
            argx_extend_sources(&sources, *it);
        }
    } else {
        argx_extend_sources(&sources, help);
    }

    if(help->id == ARGX_TYPE_SWITCH) {
        So tmp_hier_val = SO;
        so_al_nl(&out, al_ws, 1);
        so_fmt_fx(&out, rice->sw_delim, 0, "  will set:\n");
        Argx_Switch *swE = array_itE(help->val.sw);
        for(Argx_Switch *sw = help->val.sw; sw < swE; ++sw) {
            so_clear(&tmp_hier_val);
            argx_so_hierarchy(&tmp_hier_val, rice, sw->argx->group_p);
            so_fmt_fx(&out, rice->sw_delim, 0, "  --> ");
            so_fmt_al(&out, rice->sw_delim.align, 0, "%.*s", SO_F(tmp_hier_val));
            so_fmt_fx(&out, rice->sw, 0, "%.*s", SO_F(sw->argx->opt));
            so_clear(&tmp_hier_val);
            if(argx_so_val_visible(sw->argx, &sw->val)) {
                //so_push(&out, ' ');
                so_fmt_fx(&out, rice->val_delim, 0, "=");
                argx_so_val(&tmp_hier_val, rice, sw->argx, &sw->val, &opts);
                so_fmt_al(&out, rice->val_delim.align, 0, "%.*s", SO_F(tmp_hier_val));
            }
            so_al_nl(&out, al_ws, 1);
        }
        so_free(&tmp_hier_val);
    }

    if(argx_is_configurable(help)) {
        if(!help->attr.is_unconfigurable) {
            so_fmt(&out, "\nsources:\n");
            if(!array_len(sources)) {
                so_fmt(&out, "  not set anywhere");
            } else {
                So *itE = array_itE(sources);
                for(So *it = sources; it < itE; ++it) {
                    so_extend(&out, so("  "));
                    so_extend(&out, *it);
                    if(it + 1 < itE) so_extend(&out, so(",\n"));
                    else if(it > sources) so_extend(&out, so(" <-- most recent one"));
                }
            }
        }
    } else {
        so_fmt(&out, "\nunconfigurable via config file");
    }

    so_print(out);

    so_println(SO);

    so_free(&out);
    vso_free(&sources);
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

