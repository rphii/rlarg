#include "arg.h"

int arg_parse_stream(struct Arg *arg, const int argc, const char **argv) {
    /* set the stream */
    arg->stream.argc = argc;
    arg->stream.argv = argv;
    arg->stream.i = 0;
    /* now parse */
    return 0;
}

void arg_parse_setref_argx(Argx *argx) {
    if(argx->ref) {
        if(argx->is_array) {
            switch(argx->id) {
                case ARGX_BOOL: {
                    array_extend(argx->val->vb, argx->ref->vb);
                } break;
                case ARGX_URI:
                case ARGX_STRING: {
                    array_extend(argx->val->vso, argx->ref->vso);
                } break;
                case ARGX_INT: {
                    array_extend(argx->val->vi, argx->ref->vi);
                } break;
                case ARGX_SIZE: {
                    array_extend(argx->val->vz, argx->ref->vz);
                } break;
                case ARGX_NONE: break;
                case ARGX_GROUP: ABORT(ERR_UNREACHABLE("case is handled separately"));
            }
        } else {
            //printff(" v %p = r %p id %u",argx->val,argx->ref,argx->id);
            switch(argx->id) {
                case ARGX_BOOL: {
                    argx->val->b = argx->ref->b;
                } break;
                case ARGX_URI:
                case ARGX_STRING: {
                    argx->val->so = argx->ref->so;
                } break;
                case ARGX_INT: {
                    argx->val->i = argx->ref->i;
                } break;
                case ARGX_SIZE: {
                    argx->val->z = argx->ref->z;
                } break;
                case ARGX_NONE: break;
                case ARGX_GROUP: ABORT(ERR_UNREACHABLE("case is handled separately"));
            }
        }
    }
}

void arg_parse_setref_group(Argx_Group *group) {
    if(!group) return;
    for(Argx **it = group->list; it < array_itE(group->list); ++it) {
        printff("setref for: %.*s",SO_F((*it)->opt));
        ASSERT_ARG(it);
        Argx *argx = *it;
        ASSERT_ARG(argx);
        if(argx->id == ARGX_GROUP) {
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
    printff(" setref done");
}

int arg_parse(struct Arg *arg, const int argc, const char **argv) {

    arg_parse_stream(arg, argc, argv);

    arg_parse_setref(arg);

    return 0;
}

