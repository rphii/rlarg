#include <rlc.h>
#include "arg.h"
#include <sys/ioctl.h>

void arg_free(struct Arg **parg) {
    if(!parg) return;
    Arg *arg = *parg;
    if(!arg) return;
    array_free_ext(arg->opts, argx_groups_free);
    argx_group_free(&arg->pos);
    argx_group_free(&arg->env);
    array_free(arg->queue);
    vso_free(&arg->builtin.sources_paths);
    vso_free(&arg->builtin.sources_content);
    vso_free(&arg->help.sub);
    so_al_cache_free(&arg->print.p_al2);
    free(arg);
    *parg = 0;
}

void arg_init_al(Arg_Rice *rice, struct Arg *arg, So_Align_Cache *alc, bool no_default) {

#define RLARG_SO_AL_CFG(x, no_default, a, b, c, d, alc) do { \
        if(!no_default) so_al_config(x, a, b, c, d, alc); \
        else            so_al_config(x, 0, 0, SIZE_MAX, 0, alc); \
    } while(0)

    const size_t bc = arg ? arg->config.bounds.c : 0;
    const size_t bo = arg ? arg->config.bounds.opt : 0;
    const size_t bd = arg ? arg->config.bounds.desc : 0;
    const size_t bm = arg ? arg->config.bounds.max : 0;

    RLARG_SO_AL_CFG(&rice->group.align,          no_default, 0,    bc,   bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->group_delim.align,    no_default, 0,    bc,   bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->hint.align,           no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->hint_delim.align,     no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->enum_set.align,       no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->enum_delim.align,     no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->enum_unset.align,     no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->flag_set.align,       no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->flag_delim.align,     no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->flag_unset.align,     no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->sw.align,             no_default, bc,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->sw_delim.align,       no_default, bc,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->sequence.align,       no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->sequence_delim.align, no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->subopt.align,         no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->subopt_delim.align,   no_default, bo,   bo+4, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->val.align,            no_default, bo+8, bo+8, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->val_delim.align,      no_default, bo+8, bo+8, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->c.align,              no_default, bc,   bc,   bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->opt.align,            no_default, bo,   bo+2, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->env.align,            no_default, bc,   bc+2, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->desc.align,           no_default, bd,   bo+6, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->pos.align,            no_default, bc,   bc+2, bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->program.align,        no_default, 0,    0,    bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->program_delim.align,  no_default, 0,    0,    bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->program_desc.align,   no_default, 0,    0,    bm, 0, alc);
    RLARG_SO_AL_CFG(&rice->whitespace,           no_default, 0,    0,    bm, 0, alc);

    if(!no_default) {
#if 1
        if(bm - bd >= 100 - bo) {
            rice->val_delim.align.i0 = bd;
            rice->val_delim.align.iNL = bd;
            rice->val.align.i0 = bd + 2;
            rice->val.align.iNL = bd + 2;
            rice->desc.align.i0 = bd;
            rice->desc.align.iNL = bd;
        }
#endif
    }
}


struct Arg *arg_new(Arg_Config *cfg) {
    Arg *result;
    NEW(Arg, result);
    result->pos = argx_group_init(result, &result->t_pos, so("positional"), ARGX_GROUP_ROOT, 0);
    result->env = argx_group_init(result, &result->t_env, so("environment"), ARGX_GROUP_ROOT, 0);

    Arg_Config *free_cfg = 0;
    if(!cfg) {
        cfg = arg_config_new();
        free_cfg = cfg;
    }

    result->config = *cfg;
    arg_init_al(&result->rice, result, &result->print.p_al2, false);

    arg_config_free(&free_cfg);
    return result;
}

void arg_help(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;

    bool have_prog = so_len(arg->config.program);
    bool have_desc = so_len(arg->config.description);
    bool have_epil = so_len(arg->config.epilog);

    if(have_prog) so_fmt_fx(&out, arg->rice.program, 0, "%.*s", SO_F(arg->config.program));
    if(have_desc && have_prog) so_fmt_fx(&out, arg->rice.program_delim, 0, ": ");
    if(have_desc) so_fmt_fx(&out, arg->rice.program_desc, 0, "%.*s", SO_F(arg->config.description));
    if(have_prog || have_desc) so_al_nl(&out, arg->rice.whitespace, 1);

    argx_group_fmt_help(&out, &arg->pos);
    for(Argx_Group **group = arg->opts; group < array_itE(arg->opts); ++group) {
        argx_group_fmt_help(&out, *group);
    }
    argx_group_fmt_help(&out, &arg->env);

    if(have_epil) {
        so_fmt_fx(&out, arg->rice.program_desc, 0, "%.*s", SO_F(arg->config.epilog));
        so_al_nl(&out, arg->rice.whitespace, 1);
    }

    so_print(out);
    so_free(&out);
}

void arg_help_short(struct Arg *arg) {
    ASSERT_ARG(arg);
    So out = SO;
    /* format long options */
    so_al_cache_clear(&arg->print.p_al2);
    so_fmt(&out, "Options:\n");
    for(Argx_Group **group = arg->opts; group < array_itE(arg->opts); ++group) {
        Argx **xE = array_itE((*group)->list);
        for(Argx **xI = (*group)->list; xI < xE; ++xI) {
            so_fmt_fx(&out, arg->rice.opt, 0, "--%.*s", SO_F((*xI)->opt));
            so_fmt_al(&out, arg->rice.whitespace, 0, " ");
        }
    }
    
    /* format short options, TODO: if no short opts, it will still print the - without anything.. */
    so_al_cache_clear(&arg->print.p_al2);
    so_fmt(&out, "\n\nShort options:\n");
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
    so_fmt_fx(out, argx->group_p->arg->rice.opt, 0, "\n");
}

void arg_help_argx_group(struct Argx_Group *group) {
    ASSERT_ARG(group);
    So out = SO;
    argx_group_fmt_help(&out, group);
    so_print(out);
    so_free(&out);
}


void arg_stream_sources_sort(Arg_Stream_Source *arr) {
    int n = array_len(arr);

    // Start with a large gap, then reduce it step by step
    for (int gap = n / 2; gap > 0; gap /= 2) {

        // Perform a "gapped" insertion sort for this gap size
        for (int i = gap; i < n; i++) {
            
            // Current element to be placed correctly
            Arg_Stream_Source temp = arr[i];  
            int j = i;

            // Shift elements that are greater than temp to make space
            while (j >= gap && arr[j - gap].nb_source > temp.nb_source) {
                arr[j] = arr[j - gap];
                j -= gap;
            }

            // Place temp in its correct location
            arr[j] = temp;
        }
    }
}

void arg_stream_source_so(So *out, Arg_Stream_Source *src) {
    ASSERT_ARG(src);
    switch(src->id) {
        case ARG_STREAM_SOURCE_CONFIG:
            so_fmt(out, "%.*s:%u", SO_F(src->path), src->number);
            break;
        case ARG_STREAM_SOURCE_STDIN:
            so_fmt(out, "stdin@%u", src->number);
            break;
        case ARG_STREAM_SOURCE_ENVVARS:
            so_fmt(out, "envvars");
            break;
        case ARG_STREAM_SOURCE_REFVAL:
            so_fmt(out, "refval");
            break;
        default: break;
    }
}

void argx_extend_sources(Arg_Stream_Source **srces, Argx *argx) {

    ASSERT_ARG(argx);
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(argx->group_p->arg);

    size_t len = array_len(argx->sources);
    if(len) {
        for(size_t i = 0; i < len; ++i) {
            Arg_Stream_Source src = array_at(argx->sources, i);
            if(!src.id) continue;
            array_push(*srces, src);
        }
    }
}

void arg_help_argx(struct Argx *help) {
    So out = SO;
    So source_hierarchy = SO;
    Arg_Stream_Source *sources = 0;
    Argx_So_Options opts = {0};
    ASSERT_ARG(help->group_p);
    ASSERT_ARG(help->group_p->arg);
    Arg_Rice *rice = &help->group_p->arg->rice;
    So_Align al_ws = rice->whitespace;
    bool full_help = true;

    Arg_Rice dummy = {0};
    argx_so_hierarchy(&source_hierarchy, &dummy, help->group_p);
    argx_so_hierarchy(&out, rice, help->group_p);
    //so_fmt_al(&out, rice->group.align, 0, "%.*s", SO_F(xso.hierarchy));
    so_fmt_fx(&out, rice->group, 0, "%.*s", SO_F(help->opt));
    so_fmt_fx(&out, rice->group_delim, 0, ":");
    so_al_nl(&out, al_ws, 1);
    //argx_so_free(&xso);

    arg_help_argx_rec(&out, help, full_help);
    if(help->id == ARGX_TYPE_GROUP) {
        ASSERT_ARG(help->group_s);
        Argx **itE = array_itE(help->group_s->list);
        for(Argx **it = help->group_s->list; it < itE; ++it) {
            so_fmt_fx(&out, rice->opt, 0, "\n");
            argx_fmt_help(&out, *it, true);
            argx_extend_sources(&sources, *it);
        }
    } else {
        argx_extend_sources(&sources, help);
    }

    if(help->id == ARGX_TYPE_SWITCH) {
        So tmp_hier_val = SO;
        so_al_nl(&out, al_ws, 1);
        so_fmt_fx(&out, rice->sw_delim, 0, "will set:");
        so_al_nl(&out, al_ws, 1);
        Argx_Switch *swE = array_itE(help->val.sw);
        for(Argx_Switch *sw = help->val.sw; sw < swE; ++sw) {
            so_clear(&tmp_hier_val);
            argx_so_hierarchy(&tmp_hier_val, rice, sw->argx->group_p);
            so_al_cache_rewind(al_ws.cache);
            so_fmt_fx(&out, rice->sw_delim, 0, "--> ");
            so_fmt_al(&out, rice->sw_delim.align, 0, "%.*s", SO_F(tmp_hier_val));
            so_fmt_fx(&out, rice->sw, 0, "%.*s", SO_F(sw->argx->opt));
            so_clear(&tmp_hier_val);
            if(argx_so_val_visible(sw->argx, &sw->val)) {
                so_fmt_fx(&out, rice->val_delim, al_ws.cache->progress + 1, "=");
                argx_so_val(&tmp_hier_val, rice, sw->argx, &sw->val, &opts);
                so_fmt_al(&out, rice->val_delim.align, 0, "%.*s", SO_F(tmp_hier_val));
            }
            so_al_nl(&out, al_ws, 1);
        }
        so_free(&tmp_hier_val);
    }

    arg_stream_sources_sort(sources);

    if(argx_is_configurable(help)) {
        if(!help->attr.is_unconfigurable) {
            so_fmt(&out, "\nsources:\n");
            if(!array_len(sources)) {
                so_fmt(&out, "  not set anywhere");
            } else {
                Arg_Stream_Source *itE = array_itE(sources);
                for(Arg_Stream_Source *it = sources; it < itE; ++it) {
                    so_extend(&out, so("  "));
                    arg_stream_source_so(&out, it);
                    so_fmt(&out, "   ( %.*s%.*s )", SO_F(source_hierarchy), SO_F(help->opt));
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
    so_free(&source_hierarchy);
    array_free(sources);
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

struct Arg_Config *arg_config_new(void) {
    Arg_Config *cfg;
    NEW(Arg_Config, cfg);
    cfg->bounds.c = 2;
    cfg->bounds.opt = 8;
    cfg->bounds.desc = 62;
    cfg->bounds.max = 100;
    return cfg;
}

void arg_config_set_program(struct Arg_Config *cfg, So program) {
    ASSERT_ARG(cfg);
    cfg->program = program;
}

void arg_config_set_description(struct Arg_Config *cfg, So desc) {
    ASSERT_ARG(cfg);
    cfg->description = desc;
}

void arg_config_set_epilog(struct Arg_Config *cfg, So epilog) {
    ASSERT_ARG(cfg);
    cfg->epilog = epilog;
}

void arg_config_set_width(struct Arg_Config *cfg, size_t width) {
    struct winsize termsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &termsize);
    cfg->bounds.max = termsize.ws_col;
    size_t w_use = termsize.ws_col ? termsize.ws_col : 100;
    if(width == 0 || w_use < width) width = w_use;
    int desc = width * 45;
    if(desc % 200) desc += 200 - desc % 200;
    desc /= 100;
    cfg->bounds.desc = desc;
    cfg->bounds.max = width;
}

void arg_config_free(struct Arg_Config **pcfg) {
    if(!pcfg) return;
    Arg_Config *cfg = *pcfg;
    if(!cfg) return;
    free(cfg);
    *pcfg = 0;
}

