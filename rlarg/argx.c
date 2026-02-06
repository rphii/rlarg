#include "arg.h"
#include "argx.h"
#include "argx-group.h"

void argx_free_v(Argx argx);
void argx_free(Argx *argx);

LUT_IMPLEMENT(T_Argx, t_argx, So, BY_VAL, Argx, BY_VAL, so_hash, so_cmp, so_free_v, argx_free_v);

void argx_free_v(Argx argx) {
    argx_free(&argx);
}

void argx_free(Argx *argx) {
    //printff("free argx: %.*s",SO_F(argx->opt));
    if(argx->attr.is_array) {
        switch(argx->id) {
            default: ABORT(ERR_UNREACHABLE("unhandled id %u"), argx->id);
            case ARGX_TYPE_FLAG: ABORT(ERR_UNREACHABLE("array of FLAG unsupported (how did you reach this code?)"));
            case ARGX_TYPE_ENUM: ABORT(ERR_UNREACHABLE("array of ENUM unsupported (how did you reach this code?)"));
            case ARGX_TYPE_GROUP: ABORT(ERR_UNREACHABLE("array of GROUP unsupported (how did you reach this code?)"));
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
        if(argx->id == ARGX_TYPE_GROUP) {
            argx_group_free(argx->group_s);
        }
    }
    array_free_ext(argx->sources, so_free);
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

void argx_builtin_env_nocolor(struct Arg *arg) {
    Argx *x = argx_env(arg, so("NOCOLOR"), so("disable colors"));
    argx_type_bool(x, &arg->builtin.nocolor, 0);
}

int argx_callback_config(Argx *argx, void *user, So so) {
    arg_runtime_quit_when_all_valid(argx, true);
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
    arg_runtime_quit_when_all_valid(argx, true);
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
    return g;
}


void argx_fmt_help(So *out, Argx *argx) {
    ASSERT_ARG(out);
    ASSERT_ARG(argx);
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(argx->group_p->arg);
    Arg_Rice *rice = &argx->group_p->arg->rice;

    Argx_So xso = {0};
    argx_so(&xso, argx, false);

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

    size_t len_end_desc = spacing_desc + so_len_nfx(argx->desc) + 1; 
    //int spacing_

    /* format the name */
    if(treat_short_spacing) {
        so_extend(out, so("  "));
        if(argx->group_p == &argx->group_p->arg->env) {
            so_fmt_fx(out, rice->env, 0, "%.*s", SO_F(argx->opt));
        } else {
            so_fmt_fx(out, rice->pos, 0, "%.*s", SO_F(argx->opt));
        }
    } else {
        /* format the short opt */
        if(argx->c) {
            so_extend(out, so("  "));
            so_fmt_fx(out, rice->c, 0, "-%c", argx->c);
        } else {
            so_extend(out, so("    ")); 
        }
        if(treat_as_options) {
            so_extend(out, so("  ")); 
            so_fmt_fx(out, rice->opt, 0, "--%.*s", SO_F(argx->opt));
        } else {
            so_extend(out, so("    ")); 
            so_fmt_fx(out, rice->opt, 0, "%.*s", SO_F(argx->opt));
        }
    }

    if(xso.have_hint) {
        if(!compact_hint) so_push(out, '\n');
        so_fmt(out, "%*s%.*s", spacing_hint, "", SO_F(xso.hint));
    }

    if(!compact_desc) so_push(out, '\n');
    
    so_fmt(out, "%*s", spacing_desc, "");
    so_fmt_fx(out, rice->desc, 0, "%.*s", SO_F(argx->desc));

    if(xso.val_visible) {
        so_push(out, ' ');
        so_fmt_fx(out, rice->val_delim, 0, "=");
        so_fmt(out, "%.*s", SO_F(xso.set_val));
    }

    so_fmt(out, "\n");

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
    argx_so(&xso, argx, true);

    if(xso.val_group) {
        ASSERT(argx->id == ARGX_TYPE_GROUP && argx->group_s, "expected to have a group");
        Argx **itE = array_itE(argx->group_s->list);
        for(Argx **it = argx->group_s->list; it < itE; ++it) {
            argx_fmt_config(out, *it);
        }
    } else {
        if(xso.val_config) {
            so_fmt(out, "%.*s%.*s = %.*s", SO_F(xso.hierarchy), SO_F(argx->opt), SO_F(xso.set_val));
            if(xso.have_hint) {
                so_fmt(out, " # %.*s", SO_F(xso.hint));
            }
        } else {
            if(xso.have_hint) {
                so_fmt(out, "# %.*s%.*s = %.*s", SO_F(xso.hierarchy), SO_F(argx->opt), SO_F(xso.hint));
            } else {
                so_fmt(out, "# %.*s%.*s", SO_F(xso.hierarchy), SO_F(argx->opt));
            }
        }
        //so_fmt(out, " ### %.*s\n", SO_F(argx->desc));
        so_push(out, '\n');
    }

    argx_so_free(&xso);
}

/* TODO: this has to be reworked anyways, (removed) if I want to have +flags work as I want */
bool argx_flag_is_any_source_set(Argx *argx) {
    ASSERT_ARG(argx->group_p);
    Argx *parent = argx->group_p->parent;
    ASSERT_ARG(parent);
    ASSERT(parent->group_s == argx->group_p, "groups should really be the same");
    /* check if there are no sources in any of the associated enums */
    bool have_sources = false;
    Argx **itE = array_itE(parent->group_s->list);
    for(Argx **it = parent->group_s->list; it < itE; ++it) {
        if(!(*it)->sources) continue;
        if(arg_stream_souces_only_contains((*it)->sources, ARGX_SOURCE_REFVAL)) continue;
        have_sources = true;
        break;
    }
    return have_sources;
}

bool argx_is_configurable(Argx *argx) {
    if(!argx) return false;
    if(argx->attr.is_unconfigurable) return false;
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

