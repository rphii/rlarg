#include "arg.h"
#include "argx.h"
#include "argx-group.h"

void argx_free_v(Argx argx);
void argx_free(Argx *argx);

LUT_IMPLEMENT(T_Argx, t_argx, So, BY_VAL, Argx, BY_VAL, so_hash, so_cmp, so_free_v, argx_free_v);

void argx_free_v(Argx argx) {
    argx_free(&argx);
}

void argx_switch_free(Argx_Switch *sw) {
    if(!sw) return;
    if(!sw->val.any) return;
    free(sw->val.any);
}

void argx_free(Argx *argx) {
    //printff("free argx: %.*s",SO_F(argx->opt));
    if(argx->attr.is_array) {
        switch(argx->id) {
            default: ABORT(ERR_UNREACHABLE("unhandled id %u"), argx->id);
            case ARGX_TYPE_FLAG: ABORT(ERR_UNREACHABLE("array of FLAG unsupported (how did you reach this code?)"));
            case ARGX_TYPE_ENUM: ABORT(ERR_UNREACHABLE("array of ENUM unsupported (how did you reach this code?)"));
            case ARGX_TYPE_GROUP: ABORT(ERR_UNREACHABLE("array of GROUP unsupported (how did you reach this code?)"));
            case ARGX_TYPE_SWITCH: ABORT(ERR_UNREACHABLE("array of SWITCH unsupported (how did you reach this code?)"));
            case ARGX_TYPE_NONE: {}
            case ARGX_TYPE_REST:
            case ARGX_TYPE_URI:
            case ARGX_TYPE_STRING: {
                if(argx->val.vso) array_free(*argx->val.vso);
                if(argx->ref.vso) array_free(*argx->ref.vso);
            } break;
            case ARGX_TYPE_INT: {
                if(argx->val.vi) array_free(*argx->val.vi);
                if(argx->ref.vi) array_free(*argx->ref.vi);
            } break;
            case ARGX_TYPE_SIZE: {
                if(argx->val.vz) array_free(*argx->val.vz);
                if(argx->ref.vz) array_free(*argx->ref.vz);
            } break;
            case ARGX_TYPE_BOOL: {
                if(argx->val.vb) array_free(*argx->val.vb);
                if(argx->ref.vb) array_free(*argx->ref.vb);
            } break;
            case ARGX_TYPE_COLOR: {
                if(argx->val.vc) array_free(*argx->val.vc);
                if(argx->ref.vc) array_free(*argx->ref.vc);
            } break;
        }
    } else {
        if(argx->id == ARGX_TYPE_SWITCH) {
            array_free_ext(argx->val.sw, argx_switch_free);
        } else if(argx->id == ARGX_TYPE_GROUP) {
            argx_group_free(argx->group_s);
        }
    }
    array_free_ext(argx->sources, arg_stream_source_free);
}

void v_argx_free(V_Argx *vargs) {
    array_free(vargs);
}

struct Argx *argx_opt(struct Argx_Group *group, char c, So name, So desc) {
    ASSERT_ARG(group);
    ASSERT_ARG(group->table);
    T_Argx_KV *kv = t_argx_once(group->table, name, (Argx){0});
    if(!kv) {
        Argx *e = t_argx_get(group->table, name);
        ASSERT_ARG(e->group_p);
        ABORT("trying to register an argument that already exists: '%.*s' in group: '%.*s'", SO_F(name), SO_F(e->group_p->name));
    }
    kv->val.group_p = group;
    if(c) {
        if(c < '~' && c >= '!') {
            ASSERT_ARG(group->arg);
            size_t i = c - '!';
            Argx **dest = &group->arg->c[i];
            if(!*dest) {
                *dest = &kv->val;
            } else {
                ABORT("trying to register an argument '%.*s' with duplicate short-opt: '%c'", SO_F(name), c);
            }
        } else {
            ABORT("trying to register an argument '%.*s' with invalid short-opt: '%c' (= %#02x)", SO_F(name), c, c);
        }
    }
    kv->val.c = c;
    kv->val.opt = name;
    kv->val.desc = desc;
    array_push(group->list, &kv->val);
    return &kv->val;
}

struct Argx *argx_pos(struct Arg *arg, So name, So desc) {
    Argx *argx = argx_opt(&arg->pos, 0, name, desc);
    return argx;
}

struct Argx *argx_env(struct Arg *arg, So name, So desc) {
    Argx *argx = argx_opt(&arg->env, 0, name, desc);
    return argx;
}

void argx_builtin_env_compgen(struct Arg *arg) {
    Argx *x = argx_env(arg, so("COMPGEN_WORDLIST"), so("generate input for autocompletion"));
    argx_type_bool(x, &arg->builtin.compgen, 0);
}

static Arg_Builtin_Color_List g_color_mode_default = ARG_BUILTIN_COLOR_AUTO;

int argx_callback_color(Argx *argx, void *user, So so) {
    arg_update_color_off(argx->group_p->arg);
    return 0;
}

void argx_builtin_opt_color(struct Argx_Group *group) {
    ASSERT_ARG(group);
    Arg *arg = group->arg;
    ASSERT_ARG(arg);
    Argx *x = argx_opt(group, 0, so("color"), so("change color mode"));
    argx_callback(x, argx_callback_color, arg, ARGX_PRIORITY_IMMEDIATELY);
    Argx_Group *g = argx_group_enum(x, (int *)&arg->builtin.color, (int *)&g_color_mode_default);
    argx_enum_bind(g, ARG_BUILTIN_COLOR_AUTO, so("auto"), so("automatic color mode"));
    argx_enum_bind(g, ARG_BUILTIN_COLOR_OFF, so("off"), so("never show colors"));
    argx_enum_bind(g, ARG_BUILTIN_COLOR_ON, so("on"), so("always show colors"));
}

int argx_callback_config(Argx *argx, void *user, So so) {
    arg_runtime_quit_when_all_parsed(argx, true);
    Arg *arg = user;
    arg->builtin.config_print_selected = true;
    arg->help.last = 0;
    arg->help.error = 0;
    return 0;
}

void argx_builtin_env_config(struct Arg *arg) {
    Argx *x = argx_env(arg, so("CONFIG_PRINT"), so("generate config of certain group"));
    argx_callback(x, argx_callback_config, arg, ARGX_PRIORITY_IMMEDIATELY);
    Argx_Group *g = argx_group_flags(x);

    Argx_Group **itE = array_itE(arg->opts);
    for(Argx_Group **it = arg->opts; it < itE; ++it) {
        argx_flag(g, &(*it)->config_print, 0, (*it)->name, SO);
    }
}

int argx_callback_help(Argx *argx, void *user, So so) {
    arg_runtime_quit_when_all_parsed(argx, true);
    Arg *arg = user;
    //printff("!!!!! HELP WANTED");
    arg->help.wanted = true;
    return 0;
}

void argx_builtin_opt_help(struct Argx_Group *group) {
    Argx *x = argx_opt(group, 'h', so("help"), so("print this help"));
    argx_callback(x, argx_callback_help, group->arg, ARGX_PRIORITY_IMMEDIATELY);
    argx_type_rest(x, &group->arg->help.sub);
    group->arg->help.argx = x;
    argx_attr_configurable(x, false);
}

void argx_builtin_opt_source(struct Argx_Group *group, So uri) {
    Arg *arg = group->arg;
    if(!arg->builtin.sources_argx) {
        arg->builtin.sources_argx = argx_opt(group, 0, so("source"), so("source config files"));
        argx_type_array_uri(arg->builtin.sources_argx, &arg->builtin.sources_vso, 0);
    }
    Argx *argx = arg->builtin.sources_argx;
    vso_push(argx->val.vso, uri);
}

void argx_builtin_opt_so_fx(struct Argx *x, So_Fx *fmt, So_Fx *ref) {
    struct Argx_Group *g = 0;
    g=argx_group_options(x);
      x=argx_opt(g, 0, so("fg"), so("foreground"));
        argx_type_color(x, &fmt->fg, ref ? &ref->fg : 0);
      x=argx_opt(g, 0, so("bg"), so("background"));
        argx_type_color(x, &fmt->bg, ref ? &ref->bg : 0);
      x=argx_opt(g, 0, so("bold"), so("bold"));
        argx_type_bool(x, &fmt->bold, ref ? &ref->bold : 0);
      x=argx_opt(g, 0, so("it"), so("italic"));
        argx_type_bool(x, &fmt->italic, ref ? &ref->italic : 0);
      x=argx_opt(g, 0, so("ul"), so("underline"));
        argx_type_bool(x, &fmt->underline, ref ? &ref->underline : 0);
}

static void static_argx_builtin_set_rice(So_Fx *fx, bool *nocolor, Color *fg, Color *bg, bool bold, bool italic, bool underline) {
    fx->bold = bold;
    fx->italic = italic;
    fx->underline = underline;
    fx->nocolor = nocolor;
    fx->fg = fg ? *fg : (Color){0};
    fx->bg = bg ? *bg : (Color){0};
}

struct Argx_Group *argx_builtin_rice(struct Arg *arg) {
    struct Argx_Group *g = 0, *h = 0;
    Argx *x = 0;
    g=argx_group(arg, so("rice"));
      x=argx_opt(g, 0, so("arg"), so("look & feel of the argument parser"));
        h=argx_group_options(x);
          x=argx_opt(h, 0, so("program"), so("program name formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.program, 0);
          x=argx_opt(h, 0, so("group"), so("group name formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.group, 0);
          x=argx_opt(h, 0, so("group-delim"), so("group delimiter formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.group_delim, 0);
          x=argx_opt(h, 0, so("pos"), so("positional formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.pos, 0);
          x=argx_opt(h, 0, so("short"), so("short options formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.c, 0);
          x=argx_opt(h, 0, so("long"), so("long options formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.opt, 0);
          x=argx_opt(h, 0, so("env"), so("environment formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.env, 0);
          x=argx_opt(h, 0, so("desc"), so("description formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.desc, 0);
          x=argx_opt(h, 0, so("subopt"), so("suboption formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.subopt, 0);
          x=argx_opt(h, 0, so("subopt-delim"), so("suboption delimiter formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.subopt_delim, 0);
          x=argx_opt(h, 0, so("enum"), so("enum formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.enum_unset, 0);
          x=argx_opt(h, 0, so("enum-set"), so("enum set formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.enum_set, 0);
          x=argx_opt(h, 0, so("enum-delim"), so("enum delimiter formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.enum_delim, 0);
          x=argx_opt(h, 0, so("flag"), so("flag formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.flag_unset, 0);
          x=argx_opt(h, 0, so("flag-set"), so("flag set formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.flag_set, 0);
          x=argx_opt(h, 0, so("flag-delim"), so("flag delimiter formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.flag_delim, 0);
          x=argx_opt(h, 0, so("hint"), so("hint formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.hint, 0);
          x=argx_opt(h, 0, so("hint-delim"), so("hint delimiter formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.hint_delim, 0);
          x=argx_opt(h, 0, so("val"), so("value formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.val, 0);
          x=argx_opt(h, 0, so("val-delim"), so("value formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.val_delim, 0);
          x=argx_opt(h, 0, so("switch"), so("switch formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.sw, 0);
          x=argx_opt(h, 0, so("switch-delim"), so("switch formatting"));
            argx_builtin_opt_so_fx(x, &arg->rice.sw_delim, 0);

    bool *nofx = &arg->builtin.color_off;
    static_argx_builtin_set_rice(&arg->rice.program,        nofx, &COLOR_RGB(0xdd, 0x55, 0x55), 0, true, false, false);
    static_argx_builtin_set_rice(&arg->rice.program_delim,  nofx, &COLOR_RGB(0x66, 0x66, 0x66), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.program_desc,   nofx, &COLOR_RGB(0xdd, 0x55, 0x55), 0, false, false, false);
    static_argx_builtin_set_rice(&arg->rice.group,          nofx, &COLOR_RGB(0xcc, 0xcc, 0xcc), 0, true, false, true );
    static_argx_builtin_set_rice(&arg->rice.group_delim,    nofx, &COLOR_RGB(0x66, 0x66, 0x66), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.pos,            nofx, &COLOR_RGB(0xaa, 0x55, 0xdd), 0, false, true, false );
    static_argx_builtin_set_rice(&arg->rice.c,              nofx, &COLOR_RGB(0xb9, 0x7b, 0x97), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.opt,            nofx, &COLOR_RGB(0xb9, 0x7b, 0x97), 0, true, false, false );
    static_argx_builtin_set_rice(&arg->rice.env,            nofx, &COLOR_RGB(0x01, 0x01, 0x01), &COLOR_RGB(0x86, 0x58, 0x65), true, false, false );
    static_argx_builtin_set_rice(&arg->rice.desc,           nofx, &COLOR_RGB(0xaa, 0xaa, 0xaa), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.subopt,         nofx, &COLOR_RGB(0x33, 0x66, 0xee), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.subopt_delim,   nofx, &COLOR_RGB(0x66, 0x66, 0x66), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.enum_unset,     nofx, &COLOR_RGB(0x55, 0x55, 0xbb), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.enum_set,       nofx, &COLOR_RGB(0x77, 0x77, 0xdd), 0, true, false, true );
    static_argx_builtin_set_rice(&arg->rice.enum_delim,     nofx, &COLOR_RGB(0x66, 0x66, 0x66), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.flag_unset,     nofx, &COLOR_RGB(0xbb, 0xbb, 0x55), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.flag_set,       nofx, &COLOR_RGB(0xdd, 0xdd, 0x77), 0, true, false, true );
    static_argx_builtin_set_rice(&arg->rice.flag_delim,     nofx, &COLOR_RGB(0x66, 0x66, 0x66), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.hint,           nofx, &COLOR_RGB(0x55, 0xaa, 0xdd), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.hint_delim,     nofx, &COLOR_RGB(0x66, 0x66, 0x66), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.val,            nofx, &COLOR_RGB(0x55, 0xdd, 0x55), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.val_delim,      nofx, &COLOR_RGB(0x66, 0x66, 0x66), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.sw,             nofx, &COLOR_RGB(0xF6, 0x66, 0x66), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.sw_delim,       nofx, &COLOR_RGB(0x66, 0x66, 0x66), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.sequence,       nofx, &COLOR_RGB(0xF6, 0x66, 0xF6), 0, false, false, false );
    static_argx_builtin_set_rice(&arg->rice.sequence_delim, nofx, &COLOR_RGB(0x66, 0x66, 0x66), 0, false, false, false );
    return g;
}

void argx_fmt_help(So *out, Argx *argx, bool full_help) {
    ASSERT_ARG(out);
    ASSERT_ARG(argx);
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(argx->group_p->arg);
    Arg_Rice *rice = &argx->group_p->arg->rice;
    So_Align al_ws = argx->group_p->arg->print.whitespace;

    Argx_So xso = {0};
    Argx_So_Options opts = {0};
    if(!full_help) opts.array_max_items = 4;
    argx_so(&xso, argx, &opts);

    ASSERT_ARG(argx->group_p);
    bool treat_as_options = (argx->group_p->table == &argx->group_p->arg->t_opt);
    bool treat_short_spacing = (
            argx->group_p == &argx->group_p->arg->env
         || argx->group_p == &argx->group_p->arg->pos);

    /* aligning... gather lengths and spacing (( +1 because of spaces between things )) */
    size_t len_end_opt = (treat_short_spacing ? 2 : 8) + so_len(argx->opt); /* 8 because of this: '  -x  --' */
    bool compact_hint = !xso.have_hint || (xso.have_hint && len_end_opt < ARG_SPACING_HINT_WRAP);
    size_t spacing_hint = compact_hint
        ? 1
        : ARG_SPACING_HINT_ALTERNATE;

    size_t len_end_hint = (compact_hint ? len_end_opt + xso.have_hint : spacing_hint) + (xso.have_hint ? so_len_nfx(xso.hint) : 0);
    bool compact_desc = len_end_hint < ARG_SPACING_DESCRIPTION_DEFAULT;
    int spacing_desc = compact_desc
        ? ARG_SPACING_DESCRIPTION_DEFAULT - len_end_hint
        : ARG_SPACING_DESCRIPTION_ALTERNATE;

    spacing_desc = 0;
    spacing_hint = 0;

    /* format the name */
    if(treat_short_spacing) {
        //so_extend(out, so("  "));
        if(argx->group_p == &argx->group_p->arg->env) {
            so_fmt_fx(out, rice->env, 0, "%.*s", SO_F(argx->opt));
        } else {
            so_fmt_fx(out, rice->pos, 0, "%.*s", SO_F(argx->opt));
        }
    } else {
        /* format the short opt */
        if(argx->c) {
            so_fmt_fx(out, rice->c, 0, "-%c", argx->c);
        }
        if(treat_as_options) {
            so_fmt_fx(out, rice->opt, 0, "--%.*s", SO_F(argx->opt));
        } else {
            so_fmt_fx(out, rice->opt, 0, "%.*s", SO_F(argx->opt));
        }
    }

    if(xso.have_hint) {
        if(!compact_hint) so_al_nl(out, al_ws, 1);
        so_fmt_al(out, rice->hint.align, rice->hint.align.cache->progress + 1, "%.*s", SO_F(xso.hint));
    }

    //if(!compact_desc) so_al_nl(out, al_ws, 1);
    
    so_fmt_fx(out, rice->desc, 0, "%.*s", SO_F(argx->desc));

    if(argx_so_val_visible(argx, &argx->val)) {
        so_fmt_fx(out, rice->val_delim, al_ws.cache->progress + 1, "=");
        so_fmt_al(out, rice->val.align, 0, "%.*s", SO_F(xso.set_val));
    }

    //so_fmt_fx(out, rice->opt, 0, "\n");

    argx_so_free(&xso);

}

void argx_fmt_config(So *out, Argx *argx) {
    ASSERT_ARG(out);
    ASSERT_ARG(argx);

    /* pre-check if the type has no value */
    if(!argx_is_configurable(argx)) return;
    if(argx->attr.is_hidden) return;

    /* now format */
    Argx_So xso = {0};
    Argx_So_Options opts = {
        .is_for_config = true,
    };
    argx_so(&xso, argx, &opts);
    So hierarchy = SO;
    so_split_ch(xso.hierarchy, '.', &hierarchy);

    if(xso.val_group) {
        ASSERT(argx->id == ARGX_TYPE_GROUP && argx->group_s, "expected to have a group");
        Argx **itE = array_itE(argx->group_s->list);
        for(Argx **it = argx->group_s->list; it < itE; ++it) {
            argx_fmt_config(out, *it);
        }
    } else {
        if(xso.val_config) {
            so_fmt(out, "%.*s%.*s = %.*s", SO_F(hierarchy), SO_F(argx->opt), SO_F(xso.set_val));
            if(xso.have_hint) {
                so_fmt(out, " # %.*s", SO_F(xso.hint));
            }
        } else {
            if(xso.have_hint) {
                so_fmt(out, "# %.*s%.*s = %.*s", SO_F(hierarchy), SO_F(argx->opt), SO_F(xso.hint));
            } else {
                so_fmt(out, "# %.*s", SO_F(argx->opt));
            }
        }
        //so_fmt(out, " ### %.*s\n", SO_F(argx->desc));
        so_push(out, '\n');
    }

    argx_so_free(&xso);
}

bool argx_is_configurable(Argx *argx) {
    if(!argx) return false;
    if(argx->attr.is_unconfigurable) return false;
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(argx->group_p->arg);
    if(argx_is_subgroup_of_root(argx, &argx->group_p->arg->pos)) return false;
    if(argx_is_subgroup_of_root(argx, &argx->group_p->arg->env)) return false;
    if(argx->callback.func) return true;
    if(argx->id == ARGX_TYPE_NONE) return false;
    return true;
}

bool argx_is_subgroup_of_root(Argx *argx, struct Argx_Group *group) {
    ASSERT_ARG(argx);
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(group);
    Argx_Group *g = argx->group_p;
    for(;;) {
        if(!g->parent) break;
        if(!g->parent->group_p) break;
        g = g->parent->group_p;
    }
    return (bool)(g == group);
}

#if 0
bool argx_is_multiline_config(Argx *argx) {
    if(!argx) return false;
    //if(argx->attr.is_array) return true;
    switch(argx->id) {
        case ARGX_TYPE_STRING:
        case ARGX_TYPE_URI:
        case ARGX_TYPE_REST: /* TODO should this even be configurables ?? */
        default: return false;
    }
}
#endif

