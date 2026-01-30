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
    if(argx->is_array) {
        switch(argx->id) {
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
        }
    } else {
        if(argx->id == ARGX_TYPE_GROUP) {
            argx_group_free(argx->group_s);
        }
    }
    vso_free(&argx->sources);
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

int argx_callback_config(Argx *argx, void *user, So so) {
    arg_runtime_quit_when_all_valid(argx, true);
    Arg *arg = user;
    arg->builtin.config = true;
    return 0;
}

void argx_builtin_env_config(struct Arg *arg) {
    Argx *x = argx_env(arg, so("CONFIG_PRINT"), so("generate config if parser is left in a valid state"));
    argx_type_bool(x, &arg->builtin.compgen, 0);
    argx_callback(x, argx_callback_config, arg, ARGX_PRIORITY_WHEN_ALL_VALID);
}

int argx_callback_help(Argx *argx, void *user, So so) {
    arg_runtime_quit_when_all_valid(argx, true);
    Arg *arg = user;
    printff("!!!!! HELP WANTED");
    arg->help.wanted = true;
    arg->help.argx = argx;
    return 0;
}

void argx_builtin_opt_help(struct Argx_Group *group) {
    Argx *x = argx_opt(group, 'h', so("help"), so("print this help"));
    argx_callback(x, argx_callback_help, group->arg, ARGX_PRIORITY_IMMEDIATELY);
}

void argx_builtin_opt_source(struct Argx_Group *group, So uri) {
    Arg *arg = group->arg;
    if(!arg->builtin.sources_argx) {
        arg->builtin.sources_argx = argx_opt(group, 0, so("source"), so("source config files"));
        argx_type_array_uri(arg->builtin.sources_argx, &arg->builtin.sources_vso, &arg->builtin.sources_vso_ref);
    }
    Argx *argx = arg->builtin.sources_argx;
    vso_push(argx->ref.vso, uri);
}

void argx_so_free(Argx_So *xso) {
    if(!xso) return;
    if(!xso->argx) return;
    so_free(&xso->set_val);
    so_free(&xso->set_ref);
    so_free(&xso->hint);
    so_free(&xso->hierarchy);
}

void argx_so_clear(Argx_So *xso) {
    if(!xso) return;
    if(!xso->argx) return;
    so_clear(&xso->set_val);
    so_clear(&xso->set_ref);
    so_clear(&xso->hint);
    so_clear(&xso->hierarchy);
}

/* non vector types {{{ */

void argx_so_like_string(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val->so) so_fmt(out, "'%.*s'", SO_F(*val->so));
}

void argx_so_type_int(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val->i) so_fmt(out, "%d", *val->i);
}

void argx_so_type_size(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val->z) so_fmt(out, "%zi", *val->z);
}

void argx_so_type_bool(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val->b) so_extend(out, *val->b ? so("true") : so("false"));
}

/* non vector types }}} */

/* vector types {{{ */

void argx_so_like_array_string(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    ASSERT_ARG(val);
    if(val->vso) {
        so_push(out, '[');
        So *vE = array_itE(*val->vso);
        for(So *v = *val->vso; v < vE; ++v) {
            so_push(out, '\'');
            so_extend(out, *v);
            so_push(out, '\'');
            if(v + 1 < vE) so_extend(out, so(", "));
        }
        so_push(out, ']');
    }
}

void argx_so_type_array_int(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    ASSERT_ARG(val);
    if(val->vi) {
        so_push(out, '[');
        int *vE = array_itE(*val->vi);
        for(int *v = *val->vi; v < vE; ++v) {
            so_fmt(out, "%d", *v);
            if(v + 1 < vE) so_extend(out, so(", "));
        }
        so_push(out, ']');
    }
}

void argx_so_type_array_size(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    ASSERT_ARG(val);
    if(val->vz) {
        so_push(out, '[');
        ssize_t *vE = array_itE(*val->vz);
        for(ssize_t *v = *val->vz; v < vE; ++v) {
            so_fmt(out, "%d", *v);
            if(v + 1 < vE) so_extend(out, so(", "));
        }
        so_push(out, ']');
    }
}

void argx_so_type_array_bool(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    ASSERT_ARG(val);
    if(val->vb) {
        so_push(out, '[');
        bool *vE = array_itE(*val->vb);
        for(bool *v = *val->vb; v < vE; ++v) {
            so_extend(out, *v ? so("true") : so("false"));
            if(v + 1 < vE) so_extend(out, so(", "));
        }
        so_push(out, ']');
    }
}

/* vector types }}} */

void argx_so_enum(Argx_So *xso, Argx_Fmt *fmt,  Argx *argx) {
    Argx **itE = array_itE(argx->group_s->list);
    for(Argx **it = argx->group_s->list; it < itE; ++it) {
        bool current_is_selected = false;
        /* check if iterator matches selected value */
        if(argx->val.i && *argx->val.i == (*it)->val_enum) {
            so_extend(&xso->set_val, (*it)->opt);
            current_is_selected = true;
        }
        if(argx->ref.i && *argx->ref.i == (*it)->val_enum) {
            so_extend(&xso->set_ref, (*it)->opt);
        }
        /* format hint */
        if(current_is_selected && fmt) {
            so_fmt(&xso->hint, F("%.*s", UL), SO_F((*it)->opt));
        } else {
            so_fmt(&xso->hint, "%.*s", SO_F((*it)->opt));
        }
        if(it + 1 < itE) so_push(&xso->hint, '|');
    }
}

void argx_so_flags(Argx_So *xso, Argx_Fmt *fmt, Argx *argx) {
    Argx **itE = array_itE(argx->group_s->list);
    size_t iv = 0, ir = 0;
    for(Argx **it = argx->group_s->list; it < itE; ++it) {
        bool current_is_selected = false;
        /* check if iterator matches selected value */
        if((*it)->val.b && *(*it)->val.b) {
            if(iv++) so_push(&xso->set_val, ',');
            so_extend(&xso->set_val, (*it)->opt);
            current_is_selected = true;
        }
        if((*it)->ref.b && *(*it)->ref.b) {
            if(ir++) so_push(&xso->set_ref, ',');
            so_extend(&xso->set_ref, (*it)->opt);
        }
        /* format hint */
        if(current_is_selected && fmt) {
            so_fmt(&xso->hint, F("%.*s", UL), SO_F((*it)->opt));
        } else {
            so_fmt(&xso->hint, "%.*s", SO_F((*it)->opt));
        }
        if(it + 1 < itE) so_push(&xso->hint, '|');
    }
}

void argx_so_options(Argx_So *xso, Argx_Fmt *fmt, Argx *argx) {
    Argx **itE = array_itE(argx->group_s->list);
    size_t iv = 0, ir = 0;
    for(Argx **it = argx->group_s->list; it < itE; ++it) {
        /* check if iterator matches selected value */
        if((*it)->val.b && *(*it)->val.b) {
            if(iv++) so_push(&xso->set_val, ',');
            so_extend(&xso->set_val, (*it)->opt);
        }
        if((*it)->ref.b && *(*it)->ref.b) {
            if(ir++) so_push(&xso->set_ref, ',');
            so_extend(&xso->set_ref, (*it)->opt);
        }
        /* format hint */
        so_fmt(&xso->hint, "%.*s", SO_F((*it)->opt));
        if(it + 1 < itE) so_push(&xso->hint, '|');
    }
}

void argx_so_hierarchy(So *hierarchy, Argx_Group *group) {
    if(!group) return;
    if(group->parent) argx_so_hierarchy(hierarchy, group->parent->group_p);
    so_fmt(hierarchy, "%.*s.", SO_F(group->name));
}

void argx_so(Argx_So *xso, Argx_Fmt *fmt, Argx *argx) {
    //printff("FORMATTING ARGX_SO: %.*s", SO_F(argx->opt));
    ASSERT_ARG(xso);
    ASSERT_ARG(argx);
    argx_so_clear(xso);
    /* remember the hint */
    char hint[2] = {0};
    switch(argx->hint.id) {
        case ARGX_HINT_FLAGS: {
            hint[0] = '[';
            hint[1] = ']';
        } break;
        case ARGX_HINT_OPTION: {
            hint[0] = '{';
            hint[1] = '}';
        } break;
        case ARGX_HINT_OPTIONAL: {
            hint[0] = '(';
            hint[1] = ')';
        } break;
        case ARGX_HINT_REQUIRED: {
            hint[0] = '<';
            hint[1] = '>';
        } break;
    }
    /* format the value */
    //xso->ref_visible = (bool)(argx->ref);
    xso->val_visible = (bool)(argx->val.any);
    xso->val_config = xso->val_visible;
    xso->have_hint = true;
    argx_so_hierarchy(&xso->hierarchy, argx->group_p);
    if(argx->is_array) {
        switch(argx->id) {
            case ARGX_TYPE_NONE: {
                //xso->ref_visible = false;
                xso->have_hint = false;
            } break;
            case ARGX_TYPE_BOOL: {
                argx_so_type_array_bool(&xso->set_val, &argx->val);
                argx_so_type_array_bool(&xso->set_ref, &argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_TYPE_INT: {
                argx_so_type_array_int(&xso->set_val, &argx->val);
                argx_so_type_array_int(&xso->set_ref, &argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_TYPE_SIZE: {
                argx_so_type_array_size(&xso->set_val, &argx->val);
                argx_so_type_array_size(&xso->set_ref, &argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_TYPE_REST:
            case ARGX_TYPE_URI:
            case ARGX_TYPE_STRING: {
                argx_so_like_array_string(&xso->set_val, &argx->val);
                argx_so_like_array_string(&xso->set_ref, &argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_TYPE_GROUP: {
                ABORT(ERR_UNREACHABLE("vector of GROUP is not supported, and thus you should never see this message"));
            } break;
            case ARGX_TYPE_ENUM: {
                ABORT(ERR_UNREACHABLE("vector of ENUM is not supported, and thus you should never see this message"));
            } break;
            case ARGX_TYPE_FLAG: {
                ABORT(ERR_UNREACHABLE("vector of FLAG is not supported, and thus you should never see this message"));
            } break;
        }
    } else {
        switch(argx->id) {
            case ARGX_TYPE_NONE: {
                //xso->ref_visible = false;
                xso->have_hint = false;
            } break;
            case ARGX_TYPE_FLAG:
            case ARGX_TYPE_BOOL: {
                argx_so_type_bool(&xso->set_val, &argx->val);
                argx_so_type_bool(&xso->set_ref, &argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_TYPE_INT: {
                argx_so_type_int(&xso->set_val, &argx->val);
                argx_so_type_int(&xso->set_ref, &argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_TYPE_SIZE: {
                argx_so_type_size(&xso->set_val, &argx->val);
                argx_so_type_size(&xso->set_ref, &argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_TYPE_URI:
            case ARGX_TYPE_STRING: {
                argx_so_like_string(&xso->set_val, &argx->val);
                argx_so_like_string(&xso->set_ref, &argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_TYPE_GROUP: {
                xso->have_hint = false;
                so_push(&xso->hint, hint[0]);
                //printff("SUBGROUP %p,id %u,table %p,list %p,%.*s",argx->group_s,argx->group_s->id,argx->group_s->table,argx->group_s->list,SO_F(argx->opt));
                if(argx->group_s) {
                    xso->have_hint = true;
                    switch(argx->group_s->id) {
                        case ARGX_GROUP_ENUM: {
                            argx_so_enum(xso, fmt, argx);
                            xso->val_config = (bool)(xso->set_val.len);
                        } break;
                        case ARGX_GROUP_FLAGS: {
                            argx_so_flags(xso, fmt, argx);
                            xso->val_config = (bool)(xso->set_val.len);
                        } break;
                        case ARGX_GROUP_OPTIONS: {
                            argx_so_options(xso, fmt, argx);
                            xso->val_group = true;
                        } break;
                        case ARGX_GROUP_ROOT: ABORT(ERR_UNREACHABLE("case has to be handled from the outside"));
                    }
                }
                so_push(&xso->hint, hint[1]);
            } break;
            case ARGX_TYPE_ENUM: {
                xso->have_hint = false;
            } break;
            case ARGX_TYPE_REST: {
                ABORT(ERR_UNREACHABLE("non-vector of rest is not supported, and thus you should never see this message"));
            } break;
        }
    }
    xso->argx = argx;
}

#define ARG_SPACING_HINT_WRAP               40
#define ARG_SPACING_HINT_ALTERNATE          10

#define ARG_SPACING_DESCRIPTION_DEFAULT     40
#define ARG_SPACING_DESCRIPTION_ALTERNATE   12


void argx_fmt_help(So *out, Argx *argx) {
    ASSERT_ARG(out);
    ASSERT_ARG(argx);

    Argx_So xso = {0};
    Argx_Fmt fmt = {0};
    argx_so(&xso, &fmt, argx);

    /* aligning... gather lengths and spacing (( +1 because of spaces between things )) */
    size_t len_end_opt = 8 + so_len(argx->opt); /* 8 because of this: '  -x  --' */
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
    char c = argx->c ? argx->c : ' ';
    so_fmt(out, "  %c%c", argx->c ? '-' : ' ', c);
    so_fmt(out, "  --%.*s", SO_F(argx->opt));

    if(xso.have_hint) {
        if(!compact_hint) so_push(out, '\n');
        so_fmt(out, "%*s%.*s", spacing_hint, "", SO_F(xso.hint));
    }

    if(!compact_desc) so_push(out, '\n');
    
    so_fmt(out, "%*s%.*s", spacing_desc, "", SO_F(argx->desc));

    if(xso.val_visible) {
        so_fmt(out, " =%.*s", SO_F(xso.set_val));
    }

    so_fmt(out, "\n");

    argx_so_free(&xso);

}

void argx_fmt_config(So *out, Argx *argx) {
    ASSERT_ARG(out);
    ASSERT_ARG(argx);

    Argx_So xso = {0};
    argx_so(&xso, 0, argx);

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
            so_push(out, '\n');
        } else {
            if(xso.have_hint) {
                so_fmt(out, "# %.*s%.*s = %.*s\n", SO_F(xso.hierarchy), SO_F(argx->opt), SO_F(xso.hint));
            } else {
                so_fmt(out, "# %.*s%.*s\n", SO_F(xso.hierarchy), SO_F(argx->opt));
            }
        }
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
        if(!so_cmp(*(*it)->sources, ARGX_SOURCE_REFVAL)) continue;
        have_sources = true;
        break;
    }
    return have_sources;
}

