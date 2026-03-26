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

    size_t len = array_len(argx->sources);
    So tmp = SO;
    if(len) {
        for(size_t i = 0; i < len; ++i) {
            so_zero(&tmp);
            Arg_Stream_Source src = array_at(argx->sources, i);
            switch(src.id) {
                case ARG_STREAM_SOURCE_CONFIG:
                    so_fmt(&tmp, "%.*s:%u", SO_F(src.path), src.number);
                    vso_push(srces, tmp);
                    break;
                case ARG_STREAM_SOURCE_STDIN:
                    so_fmt(&tmp, "stdin@%u", src.number);
                    vso_push(srces, tmp);
                    break;
                case ARG_STREAM_SOURCE_ENVVARS:
                    vso_push(srces, so("envvars"));
                    break;
                case ARG_STREAM_SOURCE_REFVAL:
                    vso_push(srces, so("refval"));
                    break;
                default: break;
            }
        }
    }
}

void arg_help_argx(struct Argx *help) {
    So out = SO;
    VSo sources = 0;
    Argx_So xso = {0};
    Argx_So_Options opts = {0};
    ASSERT_ARG(help->group_p);
    ASSERT_ARG(help->group_p->arg);
    Arg_Rice *rice = &help->group_p->arg->rice;
    bool full_help = true;

    argx_so(&xso, help, &opts);
    so_fmt(&out, "%.*s", SO_F(xso.hierarchy));
    so_fmt_fx(&out, rice->group, 0, "%.*s", SO_F(xso.argx->opt));
    so_fmt_fx(&out, rice->group_delim, 0, ":");
    so_push(&out, '\n');
    argx_so_free(&xso);

    arg_help_argx_rec(&out, help, full_help);
    if(help->id == ARGX_TYPE_GROUP) {
        so_push(&out, '\n');
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
        so_fmt(&out, "\n");
        so_fmt_fx(&out, rice->sw_delim, 0, "  will set:\n");
        Argx_Switch *swE = array_itE(help->val.sw);
        for(Argx_Switch *sw = help->val.sw; sw < swE; ++sw) {
            so_clear(&tmp_hier_val);
            argx_so_hierarchy(&tmp_hier_val, rice, sw->argx->group_p);
            so_fmt_fx(&out, rice->sw_delim, 0, "  --> ");
            so_fmt(&out, "%.*s", SO_F(tmp_hier_val));
            so_fmt_fx(&out, rice->sw, 0, "%.*s", SO_F(sw->argx->opt));
            so_clear(&tmp_hier_val);
            if(argx_so_val_visible(sw->argx, &sw->val)) {
                so_push(&out, ' ');
                so_fmt_fx(&out, rice->val_delim, 0, "=");
                argx_so_val(&tmp_hier_val, rice, sw->argx, &sw->val, &opts);
                so_fmt(&out, "%.*s", SO_F(tmp_hier_val));
            }
            so_fmt(&out, "\n");
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
                    else so_extend(&out, so(" <-- most recent one"));
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

