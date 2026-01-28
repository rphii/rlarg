#include "arg.h"

void arg_parse_errmsg_no_rest_allowed(Arg_Stream *stream) {
    printf(F("Not allowed to set rest of values: ", FG_RD));
    while(stream->i < stream->argc) {
        So carg = so_l(stream->argv[stream->i]);
        printf("%.*s ", SO_F(carg));
        ++stream->i;
    }
    printf("\n");
}


int arg_parse_argx(struct Arg *arg, Arg_Stream *stream, Argx *argx, bool *set_rest) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(argx);
    ASSERT_ARG(set_rest);
    *set_rest = false;
    switch(argx->id) {
        case ARGX_TYPE_REST: {
            *set_rest = true;
        } break;
        case ARGX_TYPE_BOOL: {
            //if(so_as_yes_or_no(
        } break;
    }
    return 0;
}

int arg_parse_stream(struct Arg *arg, Arg_Stream *stream) {
    /* now parse */
    printff("parse... argc %u", stream->argc);
    So carg = SO;
    while(arg_stream_advance(stream, carg)) {
        int next = arg_stream_get_next(stream, &carg);
        if(next) return -1;
        printff(" carg: [%.*s]", SO_F(carg));
        /* determine kind of situation... */
        bool set_rest = false;
        if(stream->skip_flag_check) {
            set_rest = true;
        }
        /* now act upon deciding what situation we're in... */
        if(set_rest) {
            /* we want to set the rest? check if there are remaining positional arguments to be set */
            if(arg->i_pos < array_len(arg->pos.list)) {
                Argx *pos = array_at(arg->pos.list, arg->i_pos);
                arg_parse_argx(arg, stream, pos, &set_rest);
                ++arg->i_pos;
            }
            if(set_rest) {
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

