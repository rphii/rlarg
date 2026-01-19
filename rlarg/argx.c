#include "arg.h"
#include "argx.h"
#include "argx-group.h"

LUT_IMPLEMENT(T_Argx, t_argx, So, BY_VAL, Argx, BY_VAL, so_hash, so_cmp, so_free, 0);

void v_argx_free(V_Argx *vargs) {
    array_free(vargs);
}

struct Argx *argx(struct Argx_Group *group, char c, So name, So desc) {
    ASSERT_ARG(group);
    ASSERT_ARG(group->table);
    T_Argx_KV *kv = t_argx_once(group->table, name, (Argx){0});
    if(!kv) {
        Argx *e = t_argx_get(group->table, name);
        ASSERT_ARG(e->group);
        ABORT("trying to register an argument that already exists: '%.*s' in group: '%.*s'", SO_F(name), SO_F(e->group->name));
    }
    kv->val.group = group;
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
    so_free(&xso->ref);
    so_free(&xso->val);
    so_free(&xso->hint);
    so_free(&xso->hierarchy);
}

void argx_so_clear(Argx_So *xso) {
    if(!xso) return;
    if(!xso->argx) return;
    so_clear(&xso->ref);
    so_clear(&xso->val);
    so_clear(&xso->hint);
    so_clear(&xso->hierarchy);
}

void argx_so(Argx_So *xso, Argx *argx) {
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
    xso->ref_visible = (bool)(argx->ref);
    xso->have_hint = true;
    so_fmt(&xso->hierarchy, "%.*s.", SO_F(argx->group->name));
    switch(argx->id) {
        case ARGX_NONE: {
            xso->ref_visible = false;
            xso->have_hint = false;
        } break;
        case ARGX_BOOL: {
            if(argx->val) so_extend(&xso->val, argx->val->b ? so("true") : so("false"));
            if(argx->ref) so_extend(&xso->ref, argx->ref->b ? so("true") : so("false"));
            so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
        } break;
        case ARGX_INT: {
            if(argx->val) so_fmt(&xso->val, "%d", argx->val->i);
            if(argx->ref) so_fmt(&xso->ref, "%d", argx->ref->i);
            so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
        } break;
        case ARGX_SIZE: {
            if(argx->val) so_fmt(&xso->val, "%zi", argx->val->i);
            if(argx->ref) so_fmt(&xso->ref, "%zi", argx->ref->i);
            so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
        } break;
        case ARGX_ENUM:
        case ARGX_URI:
        case ARGX_STRING: {
            if(argx->val) so_extend(&xso->val, argx->val->so);
            if(argx->ref) so_extend(&xso->ref, argx->ref->so);
            so_fmt(&xso->hint, "%c%.*s%c", hint[0], SO_F(argx->hint.so), hint[1]);
        } break;
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
    argx_so(&xso, argx);

    /* gather lengths and spacing (( +1 because of spaces between things )) */
    size_t len_end_opt = 8 + so_len(argx->opt); /* 8 because of this: '  -x  --' */
    bool compact_hint = xso.have_hint && len_end_opt < ARG_SPACING_HINT_WRAP;
    size_t spacing_hint = compact_hint
        ? 1
        : ARG_SPACING_HINT_ALTERNATE;

    size_t len_end_hint = (compact_hint ? len_end_opt + 1 : spacing_hint) + (xso.have_hint ? so_len_nfx(xso.hint) : 0);
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

    if(xso.ref_visible) {
        so_fmt(out, " =%.*s", SO_F(xso.ref));
    }

    so_fmt(out, "\n");

    argx_so_free(&xso);

}

void argx_fmt_config(So *out, Argx *argx) {
    ASSERT_ARG(out);
    ASSERT_ARG(argx);

    Argx_So xso = {0};
    argx_so(&xso, argx);

    if(xso.ref_visible) {
        so_fmt(out, "%.*s%.*s = %.*s\n", SO_F(xso.hierarchy), SO_F(argx->opt), SO_F(xso.ref));
    } else {
        if(xso.have_hint) {
            so_fmt(out, "# %.*s%.*s = %.*s\n", SO_F(xso.hierarchy), SO_F(argx->opt), SO_F(xso.hint));
        } else {
            so_fmt(out, "# %.*s%.*s\n", SO_F(xso.hierarchy), SO_F(argx->opt));
        }
    }

    argx_so_free(&xso);
}

