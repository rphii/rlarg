#include "arg.h"

int arg_parse_stream(struct Arg *arg, Arg_Stream *stream) {
    /* now parse */
    printff("parse... argc %u", stream->argc);
    for(stream->i = 0; stream->i < stream->argc; ++stream->i) {
        So carg = so_l(stream->argv[stream->i]);
        if(!so_cmp(carg, so("--"))) {
            stream->skip_flag_check = true;
            continue;
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

    Arg_Stream stream_in = {
        .argc = argc ? argc - 1 : 0,
        .argv = argv ? argv + 1 : 0,
    };

    arg_parse_stream(arg, &stream_in);

    arg_parse_setref(arg);

    return 0;
}

