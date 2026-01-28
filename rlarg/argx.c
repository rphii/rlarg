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
            case ARGX_GROUP: ABORT(ERR_UNREACHABLE("array of groups unsupported (how did you reach this code?)"));
            case ARGX_NONE: {}
            case ARGX_URI:
            case ARGX_STRING: {
                array_free(argx->val->vso);
                array_free(argx->ref->vso);
            } break;
            case ARGX_INT: {
                array_free(argx->val->vi);
                array_free(argx->ref->vi);
            } break;
            case ARGX_SIZE: {
                array_free(argx->val->vz);
                array_free(argx->ref->vz);
            } break;
            case ARGX_BOOL: {
                array_free(argx->val->vb);
                array_free(argx->ref->vb);
            } break;
        }
    } else {
        if(argx->id == ARGX_GROUP) {
            argx_group_free(argx->group_s);
        }
    }
}

void v_argx_free(V_Argx *vargs) {
    array_free(vargs);
}

struct Argx *argx(struct Argx_Group *group, char c, So name, So desc) {
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
    if(val) so_fmt(out, "'%.*s'", SO_F(val->so));
}

void argx_so_type_int(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val) so_fmt(out, "%d", val->i);
}

void argx_so_type_size(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val) so_fmt(out, "%zi", val->z);
}

void argx_so_type_bool(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val) so_extend(out, val->b ? so("true") : so("false"));
}

/* non vector types }}} */

/* vector types {{{ */

void argx_so_like_array_string(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val) {
        so_push(out, '[');
        So *vE = array_itE(val->vso);
        for(So *v = val->vso; v < vE; ++v) {
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
    if(val) {
        so_push(out, '[');
        int *vE = array_itE(val->vi);
        for(int *v = val->vi; v < vE; ++v) {
            so_fmt(out, "%d", *v);
            if(v + 1 < vE) so_extend(out, so(", "));
        }
        so_push(out, ']');
    }
}

void argx_so_type_array_size(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val) {
        so_push(out, '[');
        ssize_t *vE = array_itE(val->vz);
        for(ssize_t *v = val->vz; v < vE; ++v) {
            so_fmt(out, "%d", *v);
            if(v + 1 < vE) so_extend(out, so(", "));
        }
        so_push(out, ']');
    }
}

void argx_so_type_array_bool(So *out, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val) {
        so_push(out, '[');
        bool *vE = array_itE(val->vb);
        for(bool *v = val->vb; v < vE; ++v) {
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
        if(argx->val && argx->val->i == (*it)->val_enum) {
            so_extend(&xso->set_val, (*it)->opt);
            current_is_selected = true;
        }
        if(argx->ref && argx->ref->i == (*it)->val_enum) {
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
        if((*it)->val && (*it)->val->b) {
            if(iv++) so_push(&xso->set_val, ',');
            so_extend(&xso->set_val, (*it)->opt);
            current_is_selected = true;
        }
        if((*it)->ref && (*it)->ref->b) {
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
        if((*it)->val && (*it)->val->b) {
            if(iv++) so_push(&xso->set_val, ',');
            so_extend(&xso->set_val, (*it)->opt);
        }
        if((*it)->ref && (*it)->ref->b) {
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
    argx_so_hierarchy(hierarchy, group->parent);
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
    xso->val_visible = (bool)(argx->val);
    xso->val_config = xso->val_visible;
    xso->have_hint = true;
    argx_so_hierarchy(&xso->hierarchy, argx->group_p);
    if(argx->is_array) {
        switch(argx->id) {
            case ARGX_NONE: {
                //xso->ref_visible = false;
                xso->have_hint = false;
            } break;
            case ARGX_BOOL: {
                argx_so_type_array_bool(&xso->set_val, argx->val);
                argx_so_type_array_bool(&xso->set_ref, argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_INT: {
                argx_so_type_array_int(&xso->set_val, argx->val);
                argx_so_type_array_int(&xso->set_ref, argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_SIZE: {
                argx_so_type_array_size(&xso->set_val, argx->val);
                argx_so_type_array_size(&xso->set_ref, argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_URI:
            case ARGX_STRING: {
                argx_so_like_array_string(&xso->set_val, argx->val);
                argx_so_like_array_string(&xso->set_ref, argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_GROUP: {
                ABORT(ERR_UNREACHABLE("vector of groups is not supported, and thus you should never see this message"));
            } break;
        }
    } else {
        switch(argx->id) {
            case ARGX_NONE: {
                //xso->ref_visible = false;
                xso->have_hint = false;
            } break;
            case ARGX_BOOL: {
                argx_so_type_bool(&xso->set_val, argx->val);
                argx_so_type_bool(&xso->set_ref, argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_INT: {
                argx_so_type_int(&xso->set_val, argx->val);
                argx_so_type_int(&xso->set_ref, argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_SIZE: {
                argx_so_type_size(&xso->set_val, argx->val);
                argx_so_type_size(&xso->set_ref, argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_URI:
            case ARGX_STRING: {
                argx_so_like_string(&xso->set_val, argx->val);
                argx_so_like_string(&xso->set_ref, argx->ref);
                so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
            } break;
            case ARGX_GROUP: {
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
        ASSERT(argx->id == ARGX_GROUP && argx->group_s, "expected to have a group");
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

