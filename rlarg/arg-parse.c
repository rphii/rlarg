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

void arg_parse_errmsg_missing_positionals(Arg *arg) {
    fprintf(stderr, F("Missing positional values, provided: %u/%zu", FG_RD), arg->i_pos, array_len(arg->pos.list));
    fprintf(stderr, "\n");
}

int arg_parse_group(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    So so_split = SO;
    So flagv = SO;
    int result = -1;
    bool done = false;
    do {
        Argx *subx = 0;
        printff("parse group of: %.*s", SO_F(argx->opt));
        ASSERT_ARG(argx->group_s);
        if(argx->group_s->id == ARGX_GROUP_FLAGS) {
            if(!so_splice(so, &so_split, ',')) break;
            flagv = so("1");
            if(so_at0(so_split) == '+') {
                so_split = so_i0(so_split, 1);
            } else if(so_at0(so_split) == '-') {
                so_split = so_i0(so_split, 1);
                flagv = so("0");
            }
        } else {
            so_split = so;
        }
        subx = t_argx_get(argx->group_s->table, so_split);
        if(subx) {
            switch(argx->group_s->id) {
                case ARGX_GROUP_ENUM: {
                    result = arg_parse_argx(arg, stream, subx, SO);
                    done = true;
                } break;
                case ARGX_GROUP_FLAGS: {
                    result = arg_parse_argx(arg, stream, subx, flagv);
                } break;
                case ARGX_GROUP_ROOT:
                case ARGX_GROUP_OPTIONS: {
                    So next = SO;
                    //if(arg_stream_get_next(stream, &next)) {
                        result = arg_parse_argx(arg, stream, subx, next);
                        done = true;
                    //} else {
                        //arg_parse_errmsg_missing_positionals(arg);
                    //}
                } break;
            }
        } else {
            arg_parse_errmsg_group_invalid_opt(stream, argx);
        }
        //if(result) rlc_trace_fatal();
        printff("result %u",result);
        if(done) break;
    } while(!result);
    return result;
}

int arg_parse_argx_vint(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = -1;
    int v;
    if(!so_as_int(so, &v, 0)) {
        array_push(argx->val->vi, v);
        vso_push(&argx->sources, stream->source);
        result = 0;
    } else {
        arg_parse_errmsg_invalid_conversion(stream, argx);
    }
    return result;
}

int arg_parse_argx_vsize(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = -1;
    ssize_t v;
    if(!so_as_ssize(so, &v, 0)) {
        array_push(argx->val->vz, v);
        vso_push(&argx->sources, stream->source);
        result = 0;
    } else {
        arg_parse_errmsg_invalid_conversion(stream, argx);
    }
    return result;
}

int arg_parse_argx_vbool(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = -1;
    bool v;
    if(!so_as_yes_or_no(so, &v)) {
        array_push(argx->val->vi, v);
        vso_push(&argx->sources, stream->source);
        result = 0;
    } else {
        arg_parse_errmsg_invalid_conversion(stream, argx);
    }
    return result;
}

int arg_parse_argx_vso(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = 0;
    vso_push(&argx->val->vso, so);
    vso_push(&argx->sources, stream->source);
    return result;
}


int arg_parse_argx_bool(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = so_as_yes_or_no(so, &argx->val->b);
    return result;
}

int arg_parse_argx_int(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = so_as_int(so, &argx->val->i, 0);
    return result;
}

int arg_parse_argx_size(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = so_as_ssize(so, &argx->val->z, 0);
    return result;
}

int arg_parse_argx_so(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    argx->val->so = so;
    return 0;
}

int arg_parse_argx_enum(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(!so.str);
    Argx *parent = argx->group_p->parent;
    ASSERT_ARG(parent);
    ASSERT_ARG(parent->val);
    parent->val->i = argx->val_enum;
    //vso_push(&parent->sources, stream->source);
    return 0;
}

int arg_parse_argx_none(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    //printff("TYPE IS NONE!!! [%.*s]",SO_F(argx->opt));
    //arg_stream_not_consumed(stream);
    return 0;
}


int arg_parse_argx_flag(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    /* check if any sources given in any of the related flags - if none, then reset all flags to zero */
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(so.str);
    Argx *parent = argx->group_p->parent;
    ASSERT_ARG(parent);
    ASSERT(parent->group_s == argx->group_p, "groups should really be the same");
    if(!parent->sources) {
        printff("RESET FLAGS %.*s", SO_F(parent->opt));
        Argx **itE = array_itE(parent->group_s->list);
        for(Argx **it = parent->group_s->list; it < itE; ++it) {
            printff("RESET FLAG %.*s", SO_F((*it)->opt));
            (*it)->val->b = false;
        }
    }
    /* now add source and set flag to true */
    if(so_as_yes_or_no(so, &argx->val->b)) {
        ABORT(ERR_UNREACHABLE("always have to succeed parsing what to set the flag to"));
    }
    //printff("SET FLAG %.*s / source: %.*s", SO_F(argx->opt), SO_F(stream->source));
    //vso_push(&parent->sources, stream->source);
    //vso_push(&argx->sources, stream->source);
    return 0;
}

int arg_parse_argx_group(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = arg_parse_group(arg, stream, argx, so);
    return result;
}

typedef int (*Arg_Parse_Argx_Callback)(Arg *, Arg_Stream *, Argx *, So);
typedef int (*Arg_Parse_Argx_Vector_Callback)(Arg *, Arg_Stream *, Argx *, So, Arg_Parse_Argx_Callback cb);

int arg_parse_argx_vector(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so, Arg_Parse_Argx_Callback cb) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(argx);
    ASSERT_ARG(cb);
    int result = -1;
    if(so_at0(so) == '[' && so_atE(so) == ']') {
        so = so_sub(so, 1, so_len(so) - 1);
        for(So sp = SO; so_splice(so, &sp, ','); ) {
            result = cb(arg, stream, argx, sp);
            if(result) break;
        }
    } else {
        result = cb(arg, stream, argx, so);
    }
    return result;
}

int arg_parse_argx_vector_none(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so, Arg_Parse_Argx_Callback cb) {
    arg_stream_not_consumed(stream);
    return 0;
}

int arg_parse_argx_vector_rest(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so, Arg_Parse_Argx_Callback cb) {
    arg_stream_not_consumed(stream);
    stream->rest = argx;
    return 0;
}

static Arg_Parse_Argx_Callback static_parse_argx_single_cbs[ARGX_TYPE__COUNT] = {
    [ARGX_TYPE_INT] = arg_parse_argx_int,
    [ARGX_TYPE_SIZE] = arg_parse_argx_size,
    [ARGX_TYPE_BOOL] = arg_parse_argx_bool,
    [ARGX_TYPE_ENUM] = arg_parse_argx_enum,
    [ARGX_TYPE_FLAG] = arg_parse_argx_flag,
    [ARGX_TYPE_NONE] = arg_parse_argx_none,
    [ARGX_TYPE_GROUP] = arg_parse_argx_group,
    [ARGX_TYPE_STRING] = arg_parse_argx_so,
    [ARGX_TYPE_REST] = 0,
};

static Arg_Parse_Argx_Callback static_parse_argx_vector_vals_cbs[ARGX_TYPE__COUNT] = {
    [ARGX_TYPE_INT] = arg_parse_argx_vint,
    [ARGX_TYPE_SIZE] = arg_parse_argx_vsize,
    [ARGX_TYPE_BOOL] = arg_parse_argx_vso,
    [ARGX_TYPE_STRING] = arg_parse_argx_vso,
    [ARGX_TYPE_REST] = 0,
    [ARGX_TYPE_NONE] = 0,
    [ARGX_TYPE_ENUM] = 0,
    [ARGX_TYPE_FLAG] = 0,
    [ARGX_TYPE_GROUP] = 0,
};

static Arg_Parse_Argx_Vector_Callback static_parse_argx_vector_cbs[ARGX_TYPE__COUNT] = {
    [ARGX_TYPE_INT] = arg_parse_argx_vector,
    [ARGX_TYPE_SIZE] = arg_parse_argx_vector,
    [ARGX_TYPE_BOOL] = arg_parse_argx_vector,
    [ARGX_TYPE_STRING] = arg_parse_argx_vector,
    [ARGX_TYPE_NONE] = arg_parse_argx_vector_none,
    [ARGX_TYPE_REST] = arg_parse_argx_vector_rest,
    [ARGX_TYPE_ENUM] = 0,
    [ARGX_TYPE_FLAG] = 0,
    [ARGX_TYPE_GROUP] = 0,
};

int arg_parse_argx_regular(struct Arg *arg, Arg_Stream *stream, Argx *argx, Argx_Value_Union *out, So so, Arg_Parse_Argx_Callback cb) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(argx);
    ASSERT_ARG(cb);
    int result = -1;
    if(!cb(arg, stream, argx, so)) {
        result = 0;
    } else {
        arg_parse_errmsg_invalid_conversion(stream, argx);
    }
    return result;
}

int arg_parse_argx(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(argx);
    int result = -1;
    if(argx->is_array) {
        if(argx->id < ARGX_TYPE__COUNT) {
            Arg_Parse_Argx_Callback cb = static_parse_argx_vector_vals_cbs[argx->id];
            Arg_Parse_Argx_Vector_Callback vcb = static_parse_argx_vector_cbs[argx->id];
            ASSERT_ARG(vcb);
            result = vcb(arg, stream, argx, so, cb);
        }
    } else {
        if(argx->sources) {
            /* TODO: ability to detect duplicate setting of values... for now just clear sources so they don't pile up */
            vso_clear(&argx->sources);
        }
        if(argx->id < ARGX_TYPE__COUNT) {
            Arg_Parse_Argx_Callback cb = static_parse_argx_single_cbs[argx->id];
            ASSERT_ARG(cb);
            result = cb(arg, stream, argx, so);
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
    ARG_STREAM_DONE,
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
        Arg_Stream_List situation = ARG_STREAM_DONE;
        if(stream->i < array_len(stream->vso)) situation = ARG_STREAM_REST;
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
                    printff("GOT POSITIONAL ARGX: %.*s", SO_F(pos->opt));
                    if(arg_parse_positional(arg, stream, pos)) {
                        arg_parse_errmsg_unhandled_positional_error(stream);
                        return -1;
                    }
                    ++arg->i_pos;
                    printff("POSITIONAL ARGX PARSED OK! -> i_pos %u", arg->i_pos);
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
                Argx *argx = t_argx_get(&arg->t_opt, opt);
                if(!argx) {
                    arg_parse_errmsg_root_invalid_opt(stream, opt);
                    return -1;
                }
                if(arg_parse_option(arg, stream, argx)) {
                    arg_parse_errmsg_unhandled_positional_error(stream);
                    return -1;
                }
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
                    if(arg_parse_option(arg, stream, argx)) {
                        arg_parse_errmsg_unhandled_positional_error(stream);
                        return -1;
                    }
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
                case ARGX_TYPE_FLAG: ABORT(ERR_UNREACHABLE("case is handled separately"));
            }
        } else {
            //printff(" v %p = r %p id %u",argx->val,argx->ref,argx->id);
            switch(argx->id) {
                arg_parse_setref_sources_mono(argx, ARGX_SOURCE_REFVAL, (bool)(argx->ref));
                case ARGX_TYPE_FLAG: {
                    /* check parent sources as well */
                    ASSERT_ARG(argx->group_p);
                    Argx *parent = argx->group_p->parent;
                    ASSERT_ARG(parent);
                    if(!parent->sources) {
                        argx->val->b = argx->ref->b;
                    }
                } break;
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
                case ARGX_GROUP_ROOT: {
                    arg_parse_setref_group(argx->group_s); 
                } break;
                case ARGX_GROUP_ENUM: {
                    if(argx->sources) continue;
                    argx->val = argx->ref; 
                } break;
            }
        } else {
            arg_parse_setref_argx(argx);
        }
    }
}

void arg_parse_setref(struct Arg *arg) {
    /* apply values from references */
    for(Argx_Group *group = arg->opts; group < array_itE(arg->opts); ++group) {
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
    arg_stream_from_stdin(&stream_in, argc, argv);
    status = arg_parse_stream(arg, &stream_in);
    arg_stream_free(&stream_in);
    return status;
}

int arg_parse(struct Arg *arg, const int argc, const char **argv) {

    int status = 0;

    if(!status) status = arg_parse_environment(arg);
    if(!status) status = arg_parse_stdin(arg, argc, argv);

    if(arg->i_pos < array_len(arg->pos.list)) {
        arg_parse_errmsg_missing_positionals(arg);
        return status = -1;
    }

    arg_parse_setref(arg);

    return status;
}

