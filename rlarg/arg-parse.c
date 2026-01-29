#include "arg-parse.h"

void arg_parse_errmsg_unhandled_positional_error(Arg_Stream *stream) {
    fprintf(stderr, F("Error occured while parsing: ", FG_RD));
    while(stream->i < array_len(stream->vso)) {
        So carg = array_at(stream->vso, stream->i);
        fprintf(stderr, "%.*s ", SO_F(carg));
        ++stream->i;
    }
    fprintf(stderr, "\n");
}

void arg_parse_errmsg_no_rest_allowed(Arg_Stream *stream) {
    fprintf(stderr, F("Not allowed to set rest of values: ", FG_RD));
    while(stream->i < array_len(stream->vso)) {
        So carg = array_at(stream->vso, stream->i);
        fprintf(stderr, "%.*s ", SO_F(carg));
        ++stream->i;
    }
    fprintf(stderr, "\n");
}

void arg_parse_errmsg_missing_value(Arg_Stream *stream, Argx *argx) {
    Argx_So xso = {0};
    argx_so(&xso, 0, argx);
    if(xso.have_hint) {
        fprintf(stderr, F("Missing value for argument: %.*s %.*s", FG_RD), SO_F(xso.argx->opt), SO_F(xso.hint));
    } else {
        fprintf(stderr, F("Missing value for argument: %.*s", FG_RD), SO_F(xso.argx->opt));
    }
    fprintf(stderr, "\n");
    argx_so_free(&xso);
}

void arg_parse_errmsg_invalid_conversion(Arg_Stream *stream, Argx *argx) {
    Argx_So xso = {0};
    argx_so(&xso, 0, argx);
    if(xso.have_hint) {
        fprintf(stderr, F("Invalid conversion for '%.*s' %.*s: %.*s", FG_RD), SO_F(xso.argx->opt), SO_F(xso.hint), SO_F(stream->carg));
    } else {
        ABORT(ERR_UNREACHABLE("if you reach this code, please tell me how you got here"));
    }
    fprintf(stderr, "\n");
    argx_so_free(&xso);
}

void arg_parse_errmsg_group_invalid_opt(Arg_Stream *stream, Argx *argx) {
    Argx_So xso = {0};
    argx_so(&xso, 0, argx);
    if(xso.have_hint) {
        fprintf(stderr, F("Option not found in '%.*s' %.*s: %.*s", FG_RD), SO_F(xso.argx->opt), SO_F(xso.hint), SO_F(stream->carg));
    } else {
        ABORT(ERR_UNREACHABLE("if you reach this code, please tell me how you got here"));
    }
    fprintf(stderr, "\n");
    argx_so_free(&xso);
}

void arg_parse_errmsg_root_invalid_opt(Arg_Stream *stream, So opt) {
    Argx_So xso = {0};
    fprintf(stderr, F("Option not found in root groups: '%.*s'", FG_RD), SO_F(opt));
    fprintf(stderr, "\n");
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
        arg_parse_errmsg_group_invalid_opt(stream, argx);
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
                arg_stream_not_consumed(stream);
                stream->rest = argx;
                result = 0;
            } break;
            case ARGX_TYPE_URI:
            case ARGX_TYPE_STRING: {
                vso_push(&argx->val->vso, so);
                vso_push(&argx->sources, stream->source);
                result = 0;
            } break;
            case ARGX_TYPE_INT: {
                int v;
                if(!so_as_int(so, &v, 0)) {
                    array_push(argx->val->vi, v);
                    vso_push(&argx->sources, stream->source);
                    result = 0;
                } else {
                    arg_parse_errmsg_invalid_conversion(stream, argx);
                }
            } break;
            case ARGX_TYPE_SIZE: {
                ssize_t v;
                if(!so_as_ssize(so, &v, 0)) {
                    array_push(argx->val->vz, v);
                    vso_push(&argx->sources, stream->source);
                    result = 0;
                } else {
                    arg_parse_errmsg_invalid_conversion(stream, argx);
                }
            } break;
            case ARGX_TYPE_BOOL: {
                bool v;
                if(!so_as_yes_or_no(so, &v)) {
                    array_push(argx->val->vb, v);
                    vso_push(&argx->sources, stream->source);
                    result = 0;
                } else {
                    arg_parse_errmsg_invalid_conversion(stream, argx);
                }
            } break;
            case ARGX_TYPE_ENUM: {
                ABORT(ERR_UNREACHABLE("vector of ENUM can not be parsed (unsupported)"));
            } break;
            case ARGX_TYPE_GROUP: {
                ABORT(ERR_UNREACHABLE("vector of GROUP can not be parsed (unsupported)"));
            } break;
            case ARGX_TYPE_NONE: {
                arg_stream_not_consumed(stream);
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
                so_copy(&argx->val->so, so);
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

int arg_parse_positional(struct Arg *arg, Arg_Stream *stream, Argx *argx) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(argx);
    int result = arg_parse_argx(arg, stream, argx, stream->carg);
    return result;
}

int arg_parse_option(struct Arg *arg, Arg_Stream *stream, Argx *argx) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(argx);
    So so = SO;
    switch(argx->id) {
        case ARGX_TYPE_NONE: break;
        default: {
            if(!arg_stream_get_next(stream, &so)) {
                arg_parse_errmsg_missing_value(stream, argx);
                return -1;
            }
        } break;
    }
    int result = arg_parse_argx(arg, stream, argx, so);
    return result;
}

typedef enum {
    ARG_STREAM_REST,
    ARG_STREAM_LONGOPT,
    ARG_STREAM_SHORTOPT,
} Arg_Stream_List;

int arg_parse_stream(struct Arg *arg, Arg_Stream *stream) {
    /* now parse */
    printff("parse... argc %u", array_len(stream->vso));
    So carg = SO;
    while(arg_stream_get_next(stream, &carg)) {
        printff(" carg: [%.*s]", SO_F(carg));
        /* determine kind of situation... */
        Arg_Stream_List situation = ARG_STREAM_REST;
        if(!stream->skip_flag_check) {
            if(!so_cmp0(carg, so("--")) && carg.len > 2) situation = ARG_STREAM_LONGOPT;
            else if(!so_cmp0(carg, so("-"))) situation = ARG_STREAM_SHORTOPT;
        }
        /* now act upon deciding what situation we're in... */
        switch(situation) {
            case ARG_STREAM_REST: {
                bool set_rest = true;
                /* we want to set the rest? check if there are remaining positional arguments to be set */
                printff("I_POS %u / %zu", arg->i_pos, array_len(arg->pos.list));
                if(arg->i_pos < array_len(arg->pos.list)) {
                    Argx *pos = array_at(arg->pos.list, arg->i_pos);
                    printff("GOT POSITIONAL ARGX");
                    if(arg_parse_positional(arg, stream, pos)) {
                        arg_parse_errmsg_unhandled_positional_error(stream);
                        return -1;
                    }
                    ++arg->i_pos;
                } else {
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
            } break;
            case ARG_STREAM_LONGOPT: {
                So opt = so_i0(carg, 2);
                Argx *argx = t_argx_get(&arg->table, opt);
                if(!argx) {
                    arg_parse_errmsg_root_invalid_opt(stream, opt);
                    return -1;
                }
                arg_parse_option(arg, stream, argx);
            } break;
            case ARG_STREAM_SHORTOPT: {
                So opts = so_i0(carg, 1);
                for(size_t i = 0; i < so_len(opts); ++i) {
                    unsigned char c = so_at(opts, i);
                    Argx *argx = 0;
                    if(c >= ARGX_SHORT_MIN && c < ARGX_SHORT_MAX) {
                        argx = arg->c[c - ARGX_SHORT_MIN];
                    }
                    if(!argx) {
                        arg_parse_errmsg_root_invalid_opt(stream, so_ll(&c, 1));
                        return -1;
                    }
                    arg_parse_option(arg, stream, argx);
                }
            } break;
        }
        if(situation ) {
        } else {
            /* grab an option */
        }
    }
    return 0;
}

#define ARGX_SOURCE_REFVAL  so("refval")
#define ARGX_SOURCE_STDIN   so("stdin")
#define ARGX_SOURCE_ENVVARS so("envvars")

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

int arg_parse_environment(struct Arg *arg) {
    /* gather environment variables */
    int status = 0;
    Argx **itE = array_itE(arg->env.list);
    So env = SO, carg = SO;
    Arg_Stream stream_env = {
        .source = ARGX_SOURCE_ENVVARS,
    };
    for(Argx **it = arg->env.list; it < itE && !status; ++it) {
        so_env_get(&env, (*it)->opt);
        printff("PARSE ENV: %.*s: [%.*s]", SO_F((*it)->opt), SO_F(env));
        arg_stream_clear(&stream_env);
        vso_push(&stream_env.vso, env);
        arg_stream_get_next(&stream_env, &carg);
        status = arg_parse_argx(arg, &stream_env, *it, carg);
    }
    arg_stream_free(&stream_env);
    return status;
}

int arg_parse_stdin(struct Arg *arg, const int argc, const char **argv) {
    int status = 0;
    /* parse stdin */
    Arg_Stream stream_in = {
        .source = ARGX_SOURCE_STDIN,
    };
    for(size_t i = 1; i < argc; ++i) {
        vso_push(&stream_in.vso, so_l(argv[i]));
    }
    status = arg_parse_stream(arg, &stream_in);
    arg_stream_free(&stream_in);
    return status;
}

int arg_parse(struct Arg *arg, const int argc, const char **argv) {

    int status = 0;

    if(!status) status = arg_parse_environment(arg);
    if(!status) status = arg_parse_stdin(arg, argc, argv);

    arg_parse_setref(arg);

    return status;
}

