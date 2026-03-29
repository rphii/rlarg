#include "argx-so.h"
#include "argx.h"
#include "argx-group.h"
#include "arg.h"

/* non vector types {{{ */

void argx_so_like_string(So *out, Arg_Rice *rice, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val->so) {
        so_fmt_fx(out, rice->val_delim, 0, "\"");
        for(size_t i = 0; i < so_len(*val->so); ++i) {
            char c = so_at(*val->so, i);
            if(c == '"') so_fmt_fx(out, rice->val, 0, "\\");
            so_fmt_fx(out, rice->val, 0, "%c", c);
        }
        so_fmt_fx(out, rice->val_delim, 0, "\"");
    }
}

void argx_so_type_int(So *out, Arg_Rice *rice, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val->i) {
        so_fmt_fx(out, rice->val, 0, "%d", *val->i);
    }
}

void argx_so_type_size(So *out, Arg_Rice *rice, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val->z) {
        so_fmt_fx(out, rice->val, 0, "%zi", *val->z);
    }
}

void argx_so_type_bool(So *out, Arg_Rice *rice, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val->b) {
        so_fmt_fx(out, rice->val, 0, "%s", *val->b ? "true" : "false");
    }
}

void argx_so_type_color(So *out, Arg_Rice *rice, Argx_Value_Union *val) {
    ASSERT_ARG(out);
    if(val->c) {
        if(rice) so_fmt_color(out, *val->c, SO_COLOR_RGB|SO_COLOR_HEX|SO_COLOR_PAREN);
        else so_fmt_color(out, *val->c, SO_COLOR_RGB|SO_COLOR_HEX|SO_COLOR_PAREN|SO_COLOR_NOFX);
    }
}

/* non vector types }}} */

/* vector types {{{ */

void argx_so_like_array_string(So *out, Arg_Rice *rice, Argx_Value_Union *val, So_Align al_ws, size_t max_items) {
    ASSERT_ARG(out);
    ASSERT_ARG(val);
    if(val->vso) {
        so_fmt_fx(out, rice->val_delim, 0, "[");
        So *vE = array_itE(*val->vso);
        So *vE2 = (max_items && array_len(*val->vso) > max_items) ? array_it(*val->vso, max_items) : vE;
        for(So *v = *val->vso; v < vE2; ++v) {
            so_al_nl(out, al_ws, 1);
            so_fmt_fx(out, rice->val_delim, rice->val_delim.align.i0 + 2, "\"");
            for(size_t i = 0; i < so_len(*v); ++i) {
                char c = so_at(*v, i);
                if(c == '"') so_fmt_fx(out, rice->val, 0, "\\");
                so_fmt_fx(out, rice->val, 0, "%c", c);
            }
            so_fmt_fx(out, rice->val_delim, 0, "\"");
            if(v + 1 < vE) so_fmt_fx(out, rice->val_delim, 0, ",");
        }
        if(vE2 < vE) {
            so_al_nl(out, al_ws, 1);
            so_fmt_fx(out, rice->val_delim, 0, "(+%zu more)", vE - vE2);
        }
        if(vE > *val->vso) so_al_nl(out, al_ws, 1);
        so_fmt_fx(out, rice->val_delim, 0, "]");
    }
}

void argx_so_type_array_int(So *out, Arg_Rice *rice, Argx_Value_Union *val, So_Align al_ws, size_t max_items) {
    ASSERT_ARG(out);
    ASSERT_ARG(val);
    if(val->vi) {
        so_fmt_fx(out, rice->val_delim, 0, "[");
        int *vE = array_itE(*val->vi);
        int *vE2 = (max_items && array_len(*val->vi) > max_items) ? array_it(*val->vi, max_items) : vE;
        for(int *v = *val->vi; v < vE2; ++v) {
            so_al_nl(out, al_ws, 1);
            so_fmt_fx(out, rice->val, rice->val.align.i0 + 2, "%d", *v);
            if(v + 1 < vE) so_fmt_fx(out, rice->val_delim, 0, ", ");
        }
        if(vE2 < vE) {
            so_al_nl(out, al_ws, 1);
            so_fmt_fx(out, rice->val_delim, 0, "(+%zu more)", vE - vE2);
        }
        if(vE > *val->vi) so_al_nl(out, al_ws, 1);
        so_fmt_fx(out, rice->val_delim, 0, "]");
    }
}

void argx_so_type_array_size(So *out, Arg_Rice *rice, Argx_Value_Union *val, So_Align al_ws, size_t max_items) {
    ASSERT_ARG(out);
    ASSERT_ARG(val);
    if(val->vz) {
        so_fmt_fx(out, rice->val_delim, 0, "[");
        ssize_t *vE = array_itE(*val->vz);
        ssize_t *vE2 = (max_items && array_len(*val->vz) > max_items) ? array_it(*val->vz, max_items) : vE;
        for(ssize_t *v = *val->vz; v < vE2; ++v) {
            so_al_nl(out, al_ws, 1);
            so_fmt_fx(out, rice->val, rice->val.align.i0 + 2, "%zi", *v);
            if(v + 1 < vE) so_fmt_fx(out, rice->val_delim, 0, ", ");
        }
        if(vE2 < vE) {
            so_al_nl(out, al_ws, 1);
            so_fmt_fx(out, rice->val_delim, 0, "(+%zu more)", vE - vE2);
        }
        if(vE > *val->vz) so_al_nl(out, al_ws, 1);
        so_fmt_fx(out, rice->val_delim, 0, "]");
    }
}

void argx_so_type_array_bool(So *out, Arg_Rice *rice, Argx_Value_Union *val, So_Align al_ws, size_t max_items) {
    ASSERT_ARG(out);
    ASSERT_ARG(val);
    if(val->vb) {
        so_fmt_fx(out, rice->val_delim, 0, "[");
        bool *vE = array_itE(*val->vb);
        bool *vE2 = (max_items && array_len(*val->vso) > max_items) ? array_it(*val->vb, max_items) : vE;
        for(bool *v = *val->vb; v < vE2; ++v) {
            so_al_nl(out, al_ws, 1);
            so_fmt_fx(out, rice->val, rice->val.align.i0 + 2, "%s", *v ? "true" : "false");
            if(v + 1 < vE) so_fmt_fx(out, rice->val_delim, 0, ", ");
        }
        if(vE2 < vE) {
            so_al_nl(out, al_ws, 1);
            so_fmt_fx(out, rice->val_delim, 0, "(+%zu more)", vE - vE2);
        }
        if(vE > *val->vb) so_al_nl(out, al_ws, 1);
        so_fmt_fx(out, rice->val_delim, 0, "]");
    }
}

void argx_so_type_array_color(So *out, Arg_Rice *rice, Argx_Value_Union *val, So_Align al_ws, size_t max_items) {
    ASSERT_ARG(out);
    ASSERT_ARG(val);
    if(val->vc) {
        so_fmt_fx(out, rice->val_delim, 0, "[");
        Color *vE = array_itE(*val->vc);
        Color *vE2 = (max_items && array_len(*val->vc) > max_items) ? array_it(*val->vc, max_items) : vE;
        for(Color *v = *val->vc; v < vE2; ++v) {
            so_al_nl(out, al_ws, 1);
            so_fmt_fx(out, rice->val, rice->val.align.i0 + 2, "");
            if(rice) so_fmt_color(out, *val->c, SO_COLOR_RGB|SO_COLOR_HEX|SO_COLOR_PAREN);
            else so_fmt_color(out, *val->c, SO_COLOR_RGB|SO_COLOR_HEX|SO_COLOR_PAREN|SO_COLOR_NOFX);
            so_fmt_fx(out, rice->val_delim, 0, ", ");
        }
        if(vE2 < vE) {
            so_al_nl(out, al_ws, 1);
            so_fmt_fx(out, rice->val_delim, 0, "(+%zu more)", vE - vE2);
        }
        if(vE > *val->vc) so_al_nl(out, al_ws, 1);
        so_fmt_fx(out, rice->val_delim, 0, "]");
    }
}

/* vector types }}} */

/* types relying on subgroup {{{ */

void argx_so_hint_enum(So *out, Arg_Rice *rice, char *hints, Argx *argx) {
    ASSERT_ARG(out);
    ASSERT_ARG(hints);
    ASSERT_ARG(argx);
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(argx->group_p->arg);
    bool is_pos = argx_is_subgroup_of_root(argx, &argx->group_p->arg->pos);
    so_fmt_fx(out, rice->enum_delim, 0, "%c", hints[0]);
    Argx **itE = array_itE(argx->group_s->list);
    for(Argx **it = argx->group_s->list; it < itE; ++it) {
        bool current_is_selected = false;
        if(argx->val.i && *argx->val.i == (*it)->attr.val_enum) {
            current_is_selected = !is_pos;
        }
        /* format hint */
        if(current_is_selected && rice) {
            so_fmt_fx(out, rice->enum_set, 0, "%.*s", SO_F((*it)->opt));
        } else {
            so_fmt_fx(out, rice->enum_unset, 0, "%.*s", SO_F((*it)->opt));
        }
        if(it + 1 < itE) so_fmt_fx(out, rice->enum_delim, 0, "|");
    }
    so_fmt_fx(out, rice->enum_delim, 0, "%c", hints[1]);
}

void argx_so_hint_flags(So *out, Arg_Rice *rice, char *hints, Argx *argx) {
    ASSERT_ARG(out);
    ASSERT_ARG(rice);
    ASSERT_ARG(hints);
    ASSERT_ARG(argx);
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(argx->group_p->arg);
    bool is_pos = argx_is_subgroup_of_root(argx, &argx->group_p->arg->pos);
    so_fmt_fx(out, rice->flag_delim, 0, "%c", hints[0]);
    Argx **itE = array_itE(argx->group_s->list);
    for(Argx **it = argx->group_s->list; it < itE; ++it) {
        bool current_is_selected = false;
        /* check if iterator matches selected value */
        if((*it)->val.b && *(*it)->val.b) {
            current_is_selected = !is_pos;
        }
        /* format hint */
        if(current_is_selected) {
            so_fmt_fx(out, rice->flag_set, 0, "%.*s", SO_F((*it)->opt));
        } else {
            so_fmt_fx(out, rice->flag_unset, 0, "%.*s", SO_F((*it)->opt));
        }
        if(it + 1 < itE) so_fmt_fx(out, rice->flag_delim, 0, "|");
    }
    so_fmt_fx(out, rice->flag_delim, 0, "%c", hints[1]);
}

void argx_so_hint_options(So *out, Arg_Rice *rice, char *hints, Argx *argx) {
    ASSERT_ARG(out);
    ASSERT_ARG(rice);
    ASSERT_ARG(hints);
    ASSERT_ARG(argx);
    so_fmt_fx(out, rice->subopt_delim, 0, "%c", hints[0]);
    Argx **itE = array_itE(argx->group_s->list);
    for(Argx **it = argx->group_s->list; it < itE; ++it) {
        /* format hint */
        so_fmt_fx(out, rice->subopt, 0, "%.*s", SO_F((*it)->opt));
        if(it + 1 < itE) so_fmt_fx(out, rice->subopt_delim, 0, "|");
    }
    so_fmt_fx(out, rice->subopt_delim, 0, "%c", hints[1]);
}

void argx_so_hint_sequence(So *out, Arg_Rice *rice, char *hints, Argx *argx) {
    ASSERT_ARG(out);
    ASSERT_ARG(rice);
    ASSERT_ARG(hints);
    ASSERT_ARG(argx);
    so_fmt_fx(out, rice->sequence_delim, 0, "%c", hints[0]);
    Argx **itE = array_itE(argx->group_s->list);
    for(Argx **it = argx->group_s->list; it < itE; ++it) {
        so_fmt_fx(out, rice->sequence, 0, "%.*s", SO_F((*it)->opt));
        if(it + 1 < itE) so_fmt_fx(out, rice->sequence_delim, 0, ",");
    }
    so_fmt_fx(out, rice->sequence_delim, 0, "%c", hints[1]);
}

/* types relying on subgroup }}} */

void argx_so_hierarchy(So *hierarchy, Arg_Rice *rice, Argx_Group *group) {
    if(!group) return;
    if(group->parent) argx_so_hierarchy(hierarchy, rice, group->parent->group_p);
    so_fmt_fx(hierarchy, rice->group, 0, "%.*s", SO_F(group->name));
    so_fmt_fx(hierarchy, rice->group_delim, 0, ".");
}

void argx_so_hint_generic(So *out, Arg_Rice *rice, char *hint, So so) {
    so_fmt_fx(out, rice->hint_delim, 0, "%c", hint[0]);
    so_fmt_fx(out, rice->hint, 0, "%.*s", SO_F(so));
    so_fmt_fx(out, rice->hint_delim, 0, "%c", hint[1]);
}

void argx_so_val(So *out, Arg_Rice *rice, Argx *argx, Argx_Value_Union *val, Argx_So_Options *opts) {

    ASSERT_ARG(opts);

    So_Align al_ws = rice->whitespace;

    if(argx->attr.is_array) {
        switch(argx->id) {
            default: ABORT(ERR_UNREACHABLE("unhandled id %u"), argx->id);
            case ARGX_TYPE_NONE: {
            } break;
            case ARGX_TYPE_COLOR: {
                argx_so_type_array_color(out, rice, val, al_ws, opts->array_max_items);
            } break;
            case ARGX_TYPE_BOOL: {
                argx_so_type_array_bool(out, rice, val, al_ws, opts->array_max_items);
            } break;
            case ARGX_TYPE_INT: {
                argx_so_type_array_int(out, rice, val, al_ws, opts->array_max_items);
            } break;
            case ARGX_TYPE_SIZE: {
                argx_so_type_array_size(out, rice, val, al_ws, opts->array_max_items);
            } break;
            case ARGX_TYPE_REST:
            case ARGX_TYPE_URI:
            case ARGX_TYPE_STRING: {
                argx_so_like_array_string(out, rice, val, al_ws, opts->array_max_items);
            } break;
            case ARGX_TYPE_SWITCH: {
                ABORT(ERR_UNREACHABLE("vector of SWITCH is not supported, and thus you should never see this message"));
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
            default: ABORT(ERR_UNREACHABLE("unhandled id %u"), argx->id);
            case ARGX_TYPE_NONE: {
            } break;
            case ARGX_TYPE_COLOR: {
                argx_so_type_color(out, rice, val);
            } break;
            case ARGX_TYPE_FLAG:
            case ARGX_TYPE_BOOL: {
                argx_so_type_bool(out, rice, val);
            } break;
            case ARGX_TYPE_INT: {
                argx_so_type_int(out, rice, val);
            } break;
            case ARGX_TYPE_SIZE: {
                argx_so_type_size(out, rice, val);
            } break;
            case ARGX_TYPE_URI:
            case ARGX_TYPE_STRING: {
                argx_so_like_string(out, rice, val);
            } break;
            case ARGX_TYPE_GROUP: {
                if(argx->group_s) {
                    switch(argx->group_s->id) {
                        case ARGX_GROUP_FLAGS: {
                            bool is_pos = argx_is_subgroup_of_root(argx, &argx->group_p->arg->pos);
                            bool first = true;
                            Argx **itE = array_itE(argx->group_s->list);
                            for(Argx **it = argx->group_s->list; it < itE; ++it) {
                                bool current_is_selected = false;
                                /* check if iterator matches selected value */
                                if((*it)->val.b && *(*it)->val.b) {
                                    current_is_selected = !is_pos;
                                }
                                if(current_is_selected) {
                                    if(!first) so_fmt_fx(out, rice->val_delim, 0, ",");
                                    so_fmt_fx(out, rice->val, 0, "%.*s", SO_F((*it)->opt));
                                    first = false;
                                }
                            }
                        } break;
                        case ARGX_GROUP_ENUM: {
                            bool is_pos = argx_is_subgroup_of_root(argx, &argx->group_p->arg->pos);
                            bool first = true;
                            Argx **itE = array_itE(argx->group_s->list);
                            for(Argx **it = argx->group_s->list; it < itE; ++it) {
                                bool current_is_selected = false;
                                /* check if iterator matches selected value */
                                if(argx->val.i && *argx->val.i == (*it)->attr.val_enum) {
                                    current_is_selected = !is_pos;
                                }
                                if(current_is_selected) {
                                    if(!first) so_fmt_fx(out, rice->val_delim, 0, ",");
                                    so_fmt_fx(out, rice->val, 0, "%.*s", SO_F((*it)->opt));
                                    first = false;
                                }
                            }
                        } break;
                        default: break;
                    }
                }
            } break;
            case ARGX_TYPE_SWITCH: {
            } break;
            case ARGX_TYPE_ENUM: {
            } break;
            case ARGX_TYPE_REST: {
                ABORT(ERR_UNREACHABLE("non-vector of rest is not supported, and thus you should never see this message"));
            } break;
        }
    }
}

bool argx_so_val_visible(Argx *argx, Argx_Value_Union *val) {
    ASSERT_ARG(argx);
    bool result = false;
    if(val) {
        /* if any value is set, value is visible */
        result = (bool)(val->any);
        /* if switch, hide */
        if(argx->id == ARGX_TYPE_SWITCH) result = false;
        /* if hidden, hide */
        if(argx->attr.is_hidden) result = false;
    }
    return result;
}

bool argx_so_hint_visible(Argx *argx, Argx_Value_Union *val) {
    if(argx->id == ARGX_TYPE_NONE) return false;
    if(argx->id == ARGX_TYPE_GROUP) return (bool)(argx->group_s);
    return true;
}

bool argx_so_val_config(Argx *argx, Argx_Value_Union *val) {
    if(argx->id == ARGX_TYPE_NONE) return false;
    if(argx->id == ARGX_TYPE_SWITCH) return false;
    return (bool)(val && val->any);
}

void argx_so_hint(So *out, Arg_Rice *rice, Argx *argx, Argx_Value_Union *val, Argx_So_Options *opts) {
    //printff("FORMATTING ARGX_SO: %.*s", SO_F(argx->opt));
    if(!argx) return;
    ASSERT_ARG(out);
    ASSERT_ARG(opts);
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(argx->group_p->arg);

    /* remember the hint */
    char hint[2] = {0};
    switch(argx->hint.id) {
        case ARGX_HINT_NONE: break;
        case ARGX_HINT_FLAGS: {
            hint[0] = '[';
            hint[1] = ']';
        } break;
        case ARGX_HINT_OPTION: {
            hint[0] = '{';
            hint[1] = '}';
        } break;
        case ARGX_HINT_ENUM: {
            hint[0] = '(';
            hint[1] = ')';
        } break;
        case ARGX_HINT_REQUIRED: {
            hint[0] = '<';
            hint[1] = '>';
        } break;
        case ARGX_HINT_SEQUENCE: {
            hint[0] = '|';
            hint[1] = '|';
        } break;
    }
    /* format the value */

    if(argx->attr.is_array) {
        switch(argx->id) {
            default: ABORT(ERR_UNREACHABLE("unhandled id %u"), argx->id);
            case ARGX_TYPE_NONE: {
            } break;
            case ARGX_TYPE_COLOR: {
                argx_so_hint_generic(out, rice, hint, argx->hint.so);
            } break;
            case ARGX_TYPE_BOOL: {
                argx_so_hint_generic(out, rice, hint, argx->hint.so);
            } break;
            case ARGX_TYPE_INT: {
                argx_so_hint_generic(out, rice, hint, argx->hint.so);
            } break;
            case ARGX_TYPE_SIZE: {
                argx_so_hint_generic(out, rice, hint, argx->hint.so);
            } break;
            case ARGX_TYPE_REST:
            case ARGX_TYPE_URI:
            case ARGX_TYPE_STRING: {
                argx_so_hint_generic(out, rice, hint, argx->hint.so);
            } break;
            case ARGX_TYPE_SWITCH: {
                ABORT(ERR_UNREACHABLE("vector of SWITCH is not supported, and thus you should never see this message"));
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
            default: ABORT(ERR_UNREACHABLE("unhandled id %u"), argx->id);
            case ARGX_TYPE_NONE: {
                //xso->have_hint = false;
            } break;
            case ARGX_TYPE_COLOR: {
                argx_so_hint_generic(out, rice, hint, argx->hint.so);
            } break;
            case ARGX_TYPE_FLAG:
            case ARGX_TYPE_BOOL: {
                argx_so_hint_generic(out, rice, hint, argx->hint.so);
            } break;
            case ARGX_TYPE_INT: {
                argx_so_hint_generic(out, rice, hint, argx->hint.so);
            } break;
            case ARGX_TYPE_SIZE: {
                argx_so_hint_generic(out, rice, hint, argx->hint.so);
            } break;
            case ARGX_TYPE_URI:
            case ARGX_TYPE_STRING: {
                argx_so_hint_generic(out, rice, hint, argx->hint.so);
            } break;
            case ARGX_TYPE_GROUP: {
                if(argx->group_s) {
                    switch(argx->group_s->id) {
                        case ARGX_GROUP_ENUM: {
                            argx_so_hint_enum(out, rice, hint, argx);
                        } break;
                        case ARGX_GROUP_FLAGS: {
                            argx_so_hint_flags(out, rice, hint, argx);
                        } break;
                        case ARGX_GROUP_OPTIONS: {
                            argx_so_hint_options(out, rice, hint, argx);
                        } break;
                        case ARGX_GROUP_SEQUENCE: {
                            argx_so_hint_sequence(out, rice, hint, argx);
                        } break;
                        case ARGX_GROUP_ROOT: ABORT(ERR_UNREACHABLE("case has to be handled from the outside"));
                    }
                }
            } break;
            case ARGX_TYPE_SWITCH: {
                size_t n_sw = array_len(argx->val.sw);
                so_fmt_fx(out, rice->sw_delim, 0, "(sets ");
                so_fmt_fx(out, rice->sw, 0, "%zux", n_sw);
                so_fmt_fx(out, rice->sw_delim, 0, " option%s)", n_sw > 1 ? "s" : "");
                /* set up config value ... by default always off */
                // TODO: xso->val_config = true;
                // TODO: so_extend(&xso->set_val, so("false"));
            } break;
            case ARGX_TYPE_ENUM: {
                //xso->have_hint = false;
            } break;
            case ARGX_TYPE_REST: {
                ABORT(ERR_UNREACHABLE("non-vector of rest is not supported, and thus you should never see this message"));
            } break;
        }
    }

}


