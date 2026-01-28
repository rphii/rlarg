#include "arg-parse.h"

void arg_parse_errmsg_unhandled_positional_error(Arg_Stream *stream) {
    printf(F("Error occured while parsing: ", FG_RD));
    while(stream->i < stream->argc) {
        So carg = so_l(stream->argv[stream->i]);
        printf("%.*s ", SO_F(carg));
        ++stream->i;
    }
    printf("\n");
}

void arg_parse_errmsg_no_rest_allowed(Arg_Stream *stream) {
    printf(F("Not allowed to set rest of values: ", FG_RD));
    while(stream->i < stream->argc) {
        So carg = so_l(stream->argv[stream->i]);
        printf("%.*s ", SO_F(carg));
        ++stream->i;
    }
    printf("\n");
}

void arg_parse_errmsg_missing_value(Arg_Stream *stream, Argx *argx) {
    Argx_So xso = {0};
    argx_so(&xso, 0, argx);
    if(xso.have_hint) {
        printf(F("Missing value for argument: %.*s %.*s", FG_RD), SO_F(xso.argx->opt), SO_F(xso.hint));
    } else {
        printf(F("Missing value for argument: %.*s", FG_RD), SO_F(xso.argx->opt));
    }
    printf("\n");
    argx_so_free(&xso);
}

void arg_parse_errmsg_invalid_conversion(Arg_Stream *stream, Argx *argx) {
    Argx_So xso = {0};
    argx_so(&xso, 0, argx);
    if(xso.have_hint) {
        printf(F("Invalid conversion for '%.*s' %.*s: %.*s", FG_RD), SO_F(xso.argx->opt), SO_F(xso.hint), SO_F(stream->carg));
    } else {
        ABORT(ERR_UNREACHABLE("if you reach this code, please tell me how you got here"));
    }
    printf("\n");
    argx_so_free(&xso);
}

void arg_parse_group_invalid_opt(Arg_Stream *stream, Argx *argx) {
    Argx_So xso = {0};
    argx_so(&xso, 0, argx);
    if(xso.have_hint) {
        printf(F("Option not found in '%.*s' %.*s: %.*s", FG_RD), SO_F(xso.argx->opt), SO_F(xso.hint), SO_F(stream->carg));
    } else {
        ABORT(ERR_UNREACHABLE("if you reach this code, please tell me how you got here"));
    }
    printf("\n");
    argx_so_free(&xso);
}

int arg_parse_group(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = -1;
    Argx *subx = 0;
    printff("parse group of: %.*s", SO_F(argx->opt));
    ASSERT_ARG(argx->group_s);
    subx = t_argx_get(argx->group_s->table, so);
    if(subx) {
        switch(argx->group_s->id) {
            case ARGX_GROUP_ENUM: {
                result = arg_parse_argx(arg, stream, subx, SO);
            } break;
            case ARGX_GROUP_FLAGS: {
                result = arg_parse_argx(arg, stream, subx, SO);
            } break;
            case ARGX_GROUP_ROOT:
            case ARGX_GROUP_OPTIONS: {
                So next = SO;
                if(!arg_stream_get_next(stream, &next)) {
                    result = arg_parse_argx(arg, stream, subx, next);
                }
            } break;
        }
    } else {
        arg_parse_group_invalid_opt(stream, argx);
    }
    return result;
}

int arg_parse_argx(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(argx);
    int result = -1;
    if(argx->is_array) {
        switch(argx->id) {
            case ARGX_TYPE_REST: {
                stream->rest = argx;
                result = 0;
            } break;
        }
    } else {
        switch(argx->id) {
            case ARGX_TYPE_REST: {
                ABORT(ERR_UNREACHABLE("argx of type REST is always an array"));
            } break;
            case ARGX_TYPE_BOOL: {
                if(!so_as_yes_or_no(so, &argx->val->b)) {
                    vso_push(&argx->sources, stream->source);
                    result = 0;
                } else {
                    arg_parse_errmsg_invalid_conversion(stream, argx);
                }
            } break;
            case ARGX_TYPE_INT: {
                if(!so_as_int(so, &argx->val->i, 0)) {
                    vso_push(&argx->sources, stream->source);
                    result = 0;
                } else {
                    arg_parse_errmsg_invalid_conversion(stream, argx);
                }
            } break;
            case ARGX_TYPE_SIZE: {
                if(!so_as_ssize(so, &argx->val->z, 0)) {
                    vso_push(&argx->sources, stream->source);
                    result = 0;
                } else {
                    arg_parse_errmsg_invalid_conversion(stream, argx);
                }
            } break;
            case ARGX_TYPE_URI:
            case ARGX_TYPE_STRING: {
                so_extend(&argx->val->so, so);
                vso_push(&argx->sources, stream->source);
                result = 0;
            } break;
            case ARGX_TYPE_GROUP: {
                result = arg_parse_group(arg, stream, argx, so);
            } break;
            case ARGX_TYPE_ENUM: {
                ASSERT_ARG(argx->group_p);
                ASSERT_ARG(!so.str);
                Argx *parent = argx->group_p->parent;
                ASSERT_ARG(parent);
                ASSERT_ARG(parent->val);
                parent->val->i = argx->val_enum;
                result = 0;
            } break;
            case ARGX_TYPE_NONE: {
                arg_stream_not_consumed(stream);
                result = 0;
            } break;
        }
    }
    return result;
}

int arg_parse_positional(struct Arg *arg, Arg_Stream *stream, Argx *argx, bool *set_rest) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(argx);
    ASSERT_ARG(set_rest);
    *set_rest = (argx->id == ARGX_TYPE_REST);
    printff("SET REST? %u", *set_rest);
    int result = arg_parse_argx(arg, stream, argx, stream->carg);
    return result;
}

int arg_parse_stream(struct Arg *arg, Arg_Stream *stream) {
    /* now parse */
    printff("parse... argc %u", stream->argc);
    So carg = SO;
    while(arg_stream_get_next(stream, &carg)) {
        printff(" carg: [%.*s]", SO_F(carg));
        /* determine kind of situation... */
        bool set_rest = false;
        if(stream->skip_flag_check) {
            set_rest = true;
        }
        /* now act upon deciding what situation we're in... */
        if(set_rest) {
            /* we want to set the rest? check if there are remaining positional arguments to be set */
            printff("I_POS %u / %zu", arg->i_pos, array_len(arg->pos.list));
            if(arg->i_pos < array_len(arg->pos.list)) {
                Argx *pos = array_at(arg->pos.list, arg->i_pos);
                printff("GOT POSITIONAL ARGX");
                if(arg_parse_positional(arg, stream, pos, &set_rest)) {
                    arg_parse_errmsg_unhandled_positional_error(stream);
                    return -1;
                }
                ++arg->i_pos;
            } 
            if(set_rest) {
                printff("SET REST!");
                /* all positional arguments are parsed, now push the resulting value to the rest! spit out an error if the user can not set the rest */
                Argx *rest = stream->rest;
                if(!rest || (rest && !rest->val)) {
                    arg_parse_errmsg_no_rest_allowed(stream);
                    return -1;
                } else {
                    ASSERT(rest->id == ARGX_TYPE_REST, "expecting to set the rest of parsed values into argx of type REST, have %u (%.*s)", rest->id, SO_F(rest->opt));
                    vso_push(&rest->val->vso, carg);
                }
            }
        } else {
            /* grab an option */
        }
    }
    return 0;
}

#define ARGX_SOURCE_REFVAL  so("refval")
#define ARGX_SOURCE_STDIN   so("stdin")

void arg_parse_setref_sources_mono(Argx *argx, So src, size_t n) {
    for(size_t i = 0; i < n; ++i) {
        vso_push(&argx->sources, src);
    }
}

void arg_parse_setref_argx(Argx *argx) {
    if(argx->sources) return; /* do not setref if it was already parsed somewhere else */
    if(argx->ref) {
        if(argx->is_array) {
            switch(argx->id) {
                case ARGX_TYPE_BOOL: {
                    array_extend(argx->val->vb, argx->ref->vb);
                    arg_parse_setref_sources_mono(argx, ARGX_SOURCE_REFVAL, array_len(argx->ref->vb));
                } break;
                case ARGX_TYPE_URI:
                case ARGX_TYPE_STRING: {
                    array_extend(argx->val->vso, argx->ref->vso);
                    arg_parse_setref_sources_mono(argx, ARGX_SOURCE_REFVAL, array_len(argx->ref->vso));
                } break;
                case ARGX_TYPE_INT: {
                    array_extend(argx->val->vi, argx->ref->vi);
                    arg_parse_setref_sources_mono(argx, ARGX_SOURCE_REFVAL, array_len(argx->ref->vi));
                } break;
                case ARGX_TYPE_SIZE: {
                    array_extend(argx->val->vz, argx->ref->vz);
                    arg_parse_setref_sources_mono(argx, ARGX_SOURCE_REFVAL, array_len(argx->ref->vz));
                } break;
                case ARGX_TYPE_NONE: break;
                case ARGX_TYPE_REST: ABORT(ERR_UNREACHABLE("case will never provide default values"));
                case ARGX_TYPE_GROUP: ABORT(ERR_UNREACHABLE("case is handled separately"));
                case ARGX_TYPE_ENUM: ABORT(ERR_UNREACHABLE("case is handled separately"));
            }
        } else {
            //printff(" v %p = r %p id %u",argx->val,argx->ref,argx->id);
            switch(argx->id) {
                arg_parse_setref_sources_mono(argx, ARGX_SOURCE_REFVAL, (bool)(argx->ref));
                case ARGX_TYPE_BOOL: {
                    argx->val->b = argx->ref->b;
                } break;
                case ARGX_TYPE_URI:
                case ARGX_TYPE_STRING: {
                    argx->val->so = argx->ref->so;
                } break;
                case ARGX_TYPE_INT: {
                    argx->val->i = argx->ref->i;
                } break;
                case ARGX_TYPE_SIZE: {
                    argx->val->z = argx->ref->z;
                } break;
                case ARGX_TYPE_NONE: break;
                case ARGX_TYPE_REST: ABORT(ERR_UNREACHABLE("case will never provide default values"));
                case ARGX_TYPE_GROUP: ABORT(ERR_UNREACHABLE("case is handled separately"));
                case ARGX_TYPE_ENUM: ABORT(ERR_UNREACHABLE("case is handled separately"));
            }
        }
    } else {
        argx->val = 0;
    }
}

void arg_parse_setref_group(Argx_Group *group) {
    if(!group) return;
    for(Argx **it = group->list; it < array_itE(group->list); ++it) {
        //printff("setref for: %.*s",SO_F((*it)->opt));
        ASSERT_ARG(it);
        Argx *argx = *it;
        ASSERT_ARG(argx);
        if(argx->id == ARGX_TYPE_GROUP) {
            if(!argx->group_s) continue;
            switch(argx->group_s->id) {
                case ARGX_GROUP_OPTIONS:
                case ARGX_GROUP_FLAGS:
                case ARGX_GROUP_ROOT: arg_parse_setref_group(argx->group_s); break;
                case ARGX_GROUP_ENUM: argx->val = argx->ref; break;
            }
        } else {
            arg_parse_setref_argx(argx);
        }
    }
}

void arg_parse_setref(struct Arg *arg) {
    /* apply values from references */
    for(Argx_Group *group = arg->groups; group < array_itE(arg->groups); ++group) {
        arg_parse_setref_group(group);
    }
}

int arg_parse(struct Arg *arg, const int argc, const char **argv) {

    int status = 0;

    Arg_Stream stream_in = {
        .argc = argc ? argc - 1 : 0,
        .argv = argv ? argv + 1 : 0,
    };

    status = arg_parse_stream(arg, &stream_in);
    if(status) return status;

    arg_parse_setref(arg);

    return 0;
}

