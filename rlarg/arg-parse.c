#include "arg-parse-config.h"
#include "arg-parse.h"
#include "arg.h"
#include "arg-compgen.h"
#include <unistd.h>

/* error messages {{{ */

void arg_parse_set_help_any(struct Arg *arg, Argx *argx) {
    ASSERT_ARG(arg);
    ASSERT_ARG(argx);
    if(arg->builtin.config_print_selected) return;
    arg->help.last = argx;
}

void arg_parse_set_help_error(struct Arg *arg, Argx *argx) {
    ASSERT_ARG(arg);
    if(!argx) return;
    if(arg->builtin.config_print_selected) return;
    //printff("SET ERROR if !%zu: %.*s",arg->help.error.i,SO_F(argx->opt));
    if(!arg->help.error) {
        arg->help.error = argx;
    }
    arg_parse_set_help_any(arg, argx);
}

#define FF(n,x,f)     (n ? x : F(x, f))

void arg_parse_error(Arg *arg, Arg_Stream *stream, Arg_Parse_Error_List id, Argx *argx) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(id);
    bool nc = arg->builtin.nocolor;
    arg->builtin.nocolor = true; /* TODO make this better -> argx_so should clear colors for error output? */
    Argx_So xso = {0};
    bool newline = false;
    if(arg->builtin.config_print_selected) return;
    //if(argx) { printff(F("ERROR %u, set %u [%.*s], help %u", FG_RD_B), id, stream->error_id, SO_F(argx->opt), arg->help.wanted); }
    //else printff(F("ERROR %u, set %u, help %u", FG_RD_B), id, stream->error_id, arg->help.wanted);
    bool handle = false;
    if(!stream->error_id) {
        if(!arg->help.wanted) handle = true;
        if(stream->is_help_lookup) handle = true;
    }
    if(handle) {
        //        || id == ARG_PARSE_ERROR_INVALID_OPTION_GROUP
        //        || id == ARG_PARSE_ERROR_INVALID_OPTION_ROOT)) {
        stream->error_id = id;
        switch(id) {
            case ARG_PARSE_ERROR_HIERARCHY_ROOT_CONFIG: /* pseudo */
            case ARG_PARSE_ERROR_HIERARCHY_TABLE_CONFIG: /* pseudo */
            case ARG_PARSE_ERROR_HIERARCHY_OPTION_CONFIG: /* pseudo */
            case ARG_PARSE_ERROR_MISSING_ARRAY_DELIM:
            case ARG_PARSE_ERROR_READ_FILE_SPECIFIC: /* pseudo */
            case ARG_PARSE_ERROR_MISSING_FILE_DELIM: /* pseudo */
                break;
            case ARG_PARSE_ERROR_INVALID_OPTION_ROOT: /* pseudo */
            case ARG_PARSE_ERROR_MISSING_SHORTOPT: /* pseudo */
            case ARG_PARSE_ERROR_NO_REST_ALLOWED:
                arg_parse_set_help_error(arg, arg->help.argx);
                argx_so(&xso, arg->help.argx, true, false);
                newline = (bool)(arg->help.argx);
                break;
            case ARG_PARSE_ERROR_UNCONFIGURABLE:
            case ARG_PARSE_ERROR_INVALID_CONVERSION:
            case ARG_PARSE_ERROR_INVALID_OPTION_GROUP:
            case ARG_PARSE_ERROR_MISSING_POSITIONAL:
            case ARG_PARSE_ERROR_MISSING_VALUE:
            case ARG_PARSE_ERROR_INVALID_STRING:
            case ARG_PARSE_ERROR_INVALID_STRING_END:
            case ARG_PARSE_ERROR_UNHANDLED_POSITIONAL:
            case ARG_PARSE_ERROR_READ_FILE_AUTO:
                arg_parse_set_help_error(arg, argx);
                argx_so(&xso, argx, true, true);
                newline = (bool)(argx);
                break;
            default: ABORT(ERR_UNREACHABLE("unhandled id: %u"), id);
        }
        if(!arg->builtin.compgen) {
            if(stream->source.line_number) {
                fprintf(stderr, FF(nc, "%.*s:%u: ", FG_MG_B BOLD), SO_F(stream->source.path), stream->source.line_number);
            } else {
                fprintf(stderr, FF(nc, "%.*s: ", FG_MG_B BOLD), SO_F(stream->source.path));
            }
            switch(id) {
                case ARG_PARSE_ERROR_UNCONFIGURABLE: {
                    fprintf(stderr, FF(nc, "Cannot configure: '%.*s', tried setting to: '%.*s'", FG_RD_B BOLD), SO_F(argx->opt), SO_F(stream->carg));
                } break;
                case ARG_PARSE_ERROR_INVALID_CONVERSION: {
                    fprintf(stderr, FF(nc, "Invalid conversion for '%.*s' %.*s: %.*s", FG_RD_B BOLD), SO_F(argx->opt), SO_F(xso.hint), SO_F(stream->carg));
                } break;
                case ARG_PARSE_ERROR_INVALID_OPTION_GROUP: {
                    fprintf(stderr, FF(nc, "Option not found in '%.*s' %.*s: %.*s", FG_RD_B BOLD), SO_F(argx->opt), SO_F(xso.hint), SO_F(stream->carg));
                } break;
                case ARG_PARSE_ERROR_INVALID_STRING: {
                    fprintf(stderr, FF(nc, "Invalid string for '%.*s': %.*s", FG_RD_B BOLD), SO_F(argx->opt), SO_F(stream->carg));
                } break;
                case ARG_PARSE_ERROR_INVALID_STRING_END: {
                    fprintf(stderr, FF(nc, "Invalid string end for '%.*s': %.*s", FG_RD_B BOLD), SO_F(argx->opt), SO_F(stream->carg));
                } break;
                case ARG_PARSE_ERROR_READ_FILE_AUTO: { /* pseudo */
                    fprintf(stderr, FF(nc, "Failed reading file: %.*s%.*s", FG_RD_B BOLD), SO_F(xso.hierarchy), SO_F(argx->opt));
                } break;
                case ARG_PARSE_ERROR_MISSING_FILE_DELIM: { /* pseudo */
                    fprintf(stderr, FF(nc, "Failed file()<-- end delimiter for %.*s: %.*s", FG_RD_B BOLD), SO_F(argx->opt), SO_F(argx->desc));
                } break;
                case ARG_PARSE_ERROR_READ_FILE_SPECIFIC: {
                    fprintf(stderr, FF(nc, "Failed reading file(%.*s) for: %.*s", FG_RD_B BOLD), SO_F(argx->desc), SO_F(argx->opt));
                } break;
                case ARG_PARSE_ERROR_INVALID_OPTION_ROOT: { /* pseudo */
                    fprintf(stderr, FF(nc, "Option not found in root groups: '%.*s'", FG_RD_B BOLD), SO_F(argx->opt));
                } break;
                case ARG_PARSE_ERROR_MISSING_SHORTOPT: { /* pseudo */
                    fprintf(stderr, FF(nc, "Missing short options, only provided with: %.*s", FG_RD_B BOLD), SO_F(argx->opt));
                } break;
                case ARG_PARSE_ERROR_HIERARCHY_TABLE_CONFIG: { /* pseudo */
                    fprintf(stderr, FF(nc, "Hierarchy reveals no such option table: %.*s", FG_RD_B BOLD), SO_F(argx->opt));
                } break;
                case ARG_PARSE_ERROR_HIERARCHY_ROOT_CONFIG: { /* pseudo */
                    fprintf(stderr, FF(nc, "Hierarchy reveals no root: %.*s", FG_RD_B BOLD), SO_F(argx->opt));
                } break;
                case ARG_PARSE_ERROR_HIERARCHY_OPTION_CONFIG: { /* pseudo */
                    fprintf(stderr, FF(nc, "Hierarchy reveals no such option: %.*s", FG_RD_B BOLD), SO_F(argx->opt));
                } break;
                case ARG_PARSE_ERROR_MISSING_POSITIONAL: {
                    fprintf(stderr, FF(nc, "Missing positional values: %.*s %.*s", FG_RD_B BOLD), SO_F(argx->opt), SO_F(xso.hint));
                } break;
                case ARG_PARSE_ERROR_MISSING_VALUE: {
                    fprintf(stderr, FF(nc, "Missing value for argument: %.*s %.*s", FG_RD_B BOLD), SO_F(argx->opt), SO_F(xso.hint));
                } break;
                case ARG_PARSE_ERROR_UNHANDLED_POSITIONAL: {
                    fprintf(stderr, FF(nc, "Unknown error occured while parsing: ", FG_RD_B BOLD));
                    while(stream->i < array_len(stream->vso)) {
                        So carg = array_at(stream->vso, stream->i);
                        fprintf(stderr, "%.*s ", SO_F(carg));
                        ++stream->i;
                    }
                } break;
                case ARG_PARSE_ERROR_NO_REST_ALLOWED: {
                    fprintf(stderr, FF(nc, "Not allowed to set rest of values: ", FG_RD_B BOLD));
                    while(stream->i < array_len(stream->vso)) {
                        So carg = array_at(stream->vso, stream->i);
                        fprintf(stderr, "%.*s ", SO_F(carg));
                        ++stream->i;
                    }
                } break;
                default: ABORT(ERR_UNREACHABLE("unhandled error: %u"), id);
            }
            fprintf(stderr, "\n");
            if(newline) fprintf(stderr, "\n");
        }
    }
    argx_so_free(&xso);
    arg->builtin.nocolor = nc;
}

/* error messages }}} */

void arg_parse_add_source(struct Argx *argx, Arg_Stream_Source source) {
    if(source.line_number) {
        source.path = so_clone(source.path);
        array_push(argx->sources, source);
    } else {
        array_push(argx->sources, source);
    }
}

/* coarse parsers {{{ */

int arg_parse_group(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    So so_split = SO;
    So flagv = SO;
    int result = -1;
    bool done = false;
    do {
        Argx *subx = 0;
        //printff("parse group of: %.*s", SO_F(argx->opt));
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
        //printff("GET SUB %.*s",SO_F(so_split));
        subx = t_argx_get(argx->group_s->table, so_split);
        if(subx) {
            arg_parse_set_help_any(arg, subx);
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
                    if(arg_stream_get_next(stream, &next, &arg->builtin.compgen_flags)) {
                        //printff("GOT NEXT");
                        result = arg_parse_argx(arg, stream, subx, next);
                        done = true;
                    } else {
                        arg_parse_error(arg, stream, ARG_PARSE_ERROR_MISSING_POSITIONAL, subx);
                    }
                } break;
            }
        } else {
            arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_OPTION_GROUP, argx);
        }
        if(done) break;
    } while(!result);
    return result;
}

/* coarse parsers }}} */

/* parsers for vector - values {{{ */

int arg_parse_argx_vint(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = -1;
    int v;
    if(!so_as_int(so, &v, 0)) {
        if(argx->val.vi) array_push(*argx->val.vi, v);
        arg_parse_add_source(argx, stream->source);
        result = 0;
    } else {
        arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_CONVERSION, argx);
    }
    return result;
}

int arg_parse_argx_vsize(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = -1;
    ssize_t v;
    if(!so_as_ssize(so, &v, 0)) {
        if(argx->val.vz) array_push(*argx->val.vz, v);
        arg_parse_add_source(argx, stream->source);
        result = 0;
    } else {
        arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_CONVERSION, argx);
    }
    return result;
}

int arg_parse_argx_vbool(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = -1;
    bool v;
    if(!so_as_yes_or_no(so, &v)) {
        if(argx->val.vi) array_push(*argx->val.vi, v);
        arg_parse_add_source(argx, stream->source);
        result = 0;
    } else {
        arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_CONVERSION, argx);
    }
    return result;
}

int arg_parse_argx_vso(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = 0;
    if(argx->val.vso) vso_push(argx->val.vso, so);
    arg_parse_add_source(argx, stream->source);
    return result;
}

int arg_parse_argx_vcolor(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = -1;
    Color v;
    if(!so_as_color(so, &v)) {
        if(argx->val.vc) array_push(*argx->val.vc, v);
        arg_parse_add_source(argx, stream->source);
        result = 0;
    } else {
        arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_CONVERSION, argx);
    }
    return result;
}

/* parsers for vector - values }}} */

/* parsers for regular - values {{{ */

int arg_parse_argx_bool(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    bool v;
    int result = so_as_yes_or_no(so, &v);
    if(argx->val.b) *argx->val.b = v;
    arg_parse_add_source(argx, stream->source);
    return result;
}

int arg_parse_argx_int(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int v;
    int result = so_as_int(so, &v, 0);
    if(argx->val.i) *argx->val.i = v;
    arg_parse_add_source(argx, stream->source);
    return result;
}

int arg_parse_argx_size(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    ssize_t v;
    int result = so_as_ssize(so, &v, 0);
    if(argx->val.z) *argx->val.z = v;
    arg_parse_add_source(argx, stream->source);
    return result;
}

int arg_parse_argx_so(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    if(argx->val.vso) *argx->val.so = so;
    arg_parse_add_source(argx, stream->source);
    return 0;
}

int arg_parse_argx_color(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    Color v;
    int result = so_as_color(so, &v);
    if(argx->val.z) *argx->val.c = v;
    arg_parse_add_source(argx, stream->source);
    return result;
}


int arg_parse_argx_enum(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    ASSERT_ARG(argx->group_p);
    ASSERT_ARG(!so.str);
    Argx *parent = argx->group_p->parent;
    ASSERT_ARG(parent);
    ASSERT_ARG(parent->val.i);
    if(parent->val.i) *parent->val.i = argx->attr.val_enum;
    arg_parse_add_source(argx, stream->source);
    arg_parse_add_source(parent, stream->source);
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
    //printff("PARSE FLAG %.*s",SO_F(argx->opt));
    /* check if there are no sources in any of the associated enums */
    bool have_sources = argx_flag_is_any_source_set(argx);
    /* act upon it */
    if(!have_sources) {
        //printff("RESET FLAGS %.*s", SO_F(parent->opt));
        Argx **itE = array_itE(parent->group_s->list);
        for(Argx **it = parent->group_s->list; it < itE; ++it) {
            //printff("RESET FLAG %.*s", SO_F((*it)->opt));
            if((*it)->val.b) *(*it)->val.b = false;
        }
    }
    /* now add source and set flag to true */
    bool flag;
    if(so_as_yes_or_no(so, &flag)) {
        ABORT(ERR_UNREACHABLE("always have to succeed parsing what to set the flag to"));
    }
    if(argx->val.b) *argx->val.b = flag;
    arg_parse_add_source(argx, stream->source);
    arg_parse_add_source(parent, stream->source);
    return 0;
}

int arg_parse_argx_group(Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    int result = arg_parse_group(arg, stream, argx, so);
    return result;
}

/* parsers for regular - values }}} */

typedef int (*Arg_Parse_Argx_Callback)(Arg *, Arg_Stream *, Argx *, So);
typedef int (*Arg_Parse_Argx_Vector_Callback)(Arg *, Arg_Stream *, Argx *, So, Arg_Parse_Argx_Callback cb);

/* parsers for vectors {{{ */

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
    return 0;
}

int arg_parse_argx_vector_rest(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so, Arg_Parse_Argx_Callback cb) {
    if(argx_is_subgroup_of_root(argx, &arg->pos)) {
        arg_stream_not_consumed(stream);
    }
    if(argx->val.vso) {
        stream->rest = argx;
    } else {
        stream->rest = 0;
    }
    return 0;
}

/* parsers for vectors }}} */

/* parser function assignments {{{ */

static Arg_Parse_Argx_Callback static_parse_argx_single_cbs[ARGX_TYPE__COUNT] = {
    [ARGX_TYPE_INT] = arg_parse_argx_int,
    [ARGX_TYPE_SIZE] = arg_parse_argx_size,
    [ARGX_TYPE_BOOL] = arg_parse_argx_bool,
    [ARGX_TYPE_ENUM] = arg_parse_argx_enum,
    [ARGX_TYPE_FLAG] = arg_parse_argx_flag,
    [ARGX_TYPE_NONE] = arg_parse_argx_none,
    [ARGX_TYPE_GROUP] = arg_parse_argx_group,
    [ARGX_TYPE_URI] = arg_parse_argx_so,
    [ARGX_TYPE_STRING] = arg_parse_argx_so,
    [ARGX_TYPE_COLOR] = arg_parse_argx_color,
    [ARGX_TYPE_REST] = 0,
};

static Arg_Parse_Argx_Callback static_parse_argx_vector_vals_cbs[ARGX_TYPE__COUNT] = {
    [ARGX_TYPE_INT] = arg_parse_argx_vint,
    [ARGX_TYPE_SIZE] = arg_parse_argx_vsize,
    [ARGX_TYPE_BOOL] = arg_parse_argx_vso,
    [ARGX_TYPE_URI] = arg_parse_argx_vso,
    [ARGX_TYPE_STRING] = arg_parse_argx_vso,
    [ARGX_TYPE_COLOR] = arg_parse_argx_vcolor,
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
    [ARGX_TYPE_URI] = arg_parse_argx_vector,
    [ARGX_TYPE_STRING] = arg_parse_argx_vector,
    [ARGX_TYPE_COLOR] = arg_parse_argx_vector,
    [ARGX_TYPE_NONE] = arg_parse_argx_vector_none,
    [ARGX_TYPE_REST] = arg_parse_argx_vector_rest,
    [ARGX_TYPE_ENUM] = 0,
    [ARGX_TYPE_FLAG] = 0,
    [ARGX_TYPE_GROUP] = 0,
};

/* parser function assignments }}} */

/* main parsing section {{{ */

int arg_parse_argx(struct Arg *arg, Arg_Stream *stream, Argx *argx, So so) {
    ASSERT_ARG(arg);
    ASSERT_ARG(stream);
    ASSERT_ARG(argx);
    //printff("PARSE: %.*s",SO_F(argx->opt));
    int result = -1;
    arg_parse_set_help_any(arg, argx); /* set help BEFORE doing any further parsing */
    if(stream->is_config && !argx_is_configurable(argx)) {
        result = -1;
        arg_parse_error(arg, stream, ARG_PARSE_ERROR_UNCONFIGURABLE, argx);
    } else if(argx->attr.is_array) {
        if(argx->id < ARGX_TYPE__COUNT) {
            Arg_Parse_Argx_Callback cb = static_parse_argx_vector_vals_cbs[argx->id];
            Arg_Parse_Argx_Vector_Callback vcb = static_parse_argx_vector_cbs[argx->id];
            ASSERT_ARG(vcb);
            result = vcb(arg, stream, argx, so, cb);
        }
    } else {
        if(argx->sources) {
            /* TODO: ability to detect duplicate setting of values... for now just clear sources so they don't pile up */
            array_clear_ext(argx->sources, so_free);
        }
        if(argx->id < ARGX_TYPE__COUNT) {
            Arg_Parse_Argx_Callback cb = static_parse_argx_single_cbs[argx->id];
            ASSERT_ARG(cb);
            result = cb(arg, stream, argx, so);
            if(result) {
                arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_CONVERSION, argx);
            }
        }
    }
    if(!result && argx->callback.func) {
        if(argx->callback.priority == ARGX_PRIORITY_IMMEDIATELY) {
            result = argx->callback.func(argx, argx->callback.user, so);
        } else {
            Argx_Callback_Queue q = { .argx = argx, .so = so };
            array_push(arg->queue, q);
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
        case ARGX_TYPE_REST: break;
        default: {
            arg_parse_set_help_any(arg, argx);
            if(!arg_stream_get_next(stream, &so, &arg->builtin.compgen_flags)) {
                arg_parse_error(arg, stream, ARG_PARSE_ERROR_MISSING_VALUE, argx);
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

Argx *arg_parse_hierarchy(struct Arg *arg, Arg_Stream *stream, So lhs, Argx_Group **root_group) {
    Argx *result = 0;

    So root = so_trim(so_split_ch(lhs, '.', &lhs));

    /* verify that the root group exists */
    bool exist = false;
    Argx_Groups groupE = array_itE(arg->opts);
    for(Argx_Groups group = arg->opts; group < groupE; ++group) {
        if(so_cmp((*group)->name, root)) continue;
        if(root_group) *root_group = *group;
        exist = true;
        break;
    }
    T_Argx *table = &arg->t_opt;
    if(!exist && stream->is_help_lookup) {
        ASSERT_ARG(root_group);
        if(!so_cmp(arg->env.name, root)) {
            *root_group = &arg->env;
            table = &arg->t_env;
            exist = true;
        }
        if(!so_cmp(arg->pos.name, root)) {
            *root_group = &arg->pos;
            table = &arg->t_pos;
            exist = true;
        }
    }
    if(!exist) {
        Argx pseudo = { .opt = root };
        arg_parse_error(arg, stream, ARG_PARSE_ERROR_HIERARCHY_ROOT_CONFIG, &pseudo);
        return 0;
    }

    /* now search for sub option */
    for(So opt = SO; so_splice(lhs, &opt, '.'); ) {
        if(!table) {
            Argx pseudo = { .opt = lhs };
            arg_parse_error(arg, stream, ARG_PARSE_ERROR_HIERARCHY_TABLE_CONFIG, &pseudo);
            return 0;
        }
        result = t_argx_get(table, opt);
        if(!result) {
            Argx pseudo = { .opt = lhs };
            arg_parse_error(arg, stream, ARG_PARSE_ERROR_HIERARCHY_TABLE_CONFIG, &pseudo);
            return 0;
        }
        table = result->group_s ? result->group_s->table : 0;
        if(table && root_group) *root_group = result->group_s;
    }

    return result;
}

int arg_parse_stream(struct Arg *arg, Arg_Stream *stream) {
    /* now parse */
    //printff("parse... argc %u", array_len(stream->vso));
    So carg = SO;
    int status = 0;
    bool get_env_help = false;
    while(arg_stream_get_next(stream, &carg, &arg->builtin.compgen_flags)) {
        //printff("carg: [%.*s], skip flag? %u", SO_F(carg), stream->skip_flag_check);
        /* determine kind of situation... */
        if(get_env_help) goto error_but_maybe_get_env_help;
        Arg_Stream_List situation = ARG_STREAM_DONE;
        if(stream->i < array_len(stream->vso)) situation = ARG_STREAM_REST;
        if(!stream->skip_flag_check && !stream->rest) {
            if(!so_cmp0(carg, so("--")) && carg.len > 2) situation = ARG_STREAM_LONGOPT;
            else if(!so_cmp0(carg, so("-"))) situation = ARG_STREAM_SHORTOPT;
        }
        //printff(" situation %u, hl %u",situation,stream->is_help_lookup);
        /* now act upon deciding what situation we're in... */
        switch(situation) {
            case ARG_STREAM_DONE: break;
            case ARG_STREAM_REST: {
                /* we want to set the rest? check if there are remaining positional arguments to be set */
                //printff("I_POS %u / %zu", arg->i_pos, array_len(arg->pos.list));
                if(arg->i_pos < array_len(arg->pos.list) && !stream->rest) {
                    Argx *pos = array_at(arg->pos.list, arg->i_pos);
                    //printff("GOT POSITIONAL ARGX: %.*s", SO_F(pos->opt));
                    if(arg_parse_positional(arg, stream, pos)) {
                        arg_parse_error(arg, stream, ARG_PARSE_ERROR_UNHANDLED_POSITIONAL, 0);
                        status = -1;
                        goto error_but_maybe_get_env_help;
                    }
                    ++arg->i_pos;
                    //printff("POSITIONAL ARGX PARSED OK! -> i_pos %u", arg->i_pos);
                } else {
                    //printff(" SET REST!");
                    /* all positional arguments are parsed, now push the resulting value to the rest! spit out an error if the user can not set the rest */
                    Argx *rest = stream->rest;
                    if(!rest || (rest && !rest->val.vso)) {
                        arg_parse_error(arg, stream, ARG_PARSE_ERROR_NO_REST_ALLOWED, 0);
                        status = -1;
                        goto error_but_maybe_get_env_help;
                    } else {
                        ASSERT(rest->id == ARGX_TYPE_REST, "expecting to set the rest of parsed values into argx of type REST, have %u (%.*s)", rest->id, SO_F(rest->opt));
                        vso_push(rest->val.vso, carg);
                    }
                }
            } break;
            case ARG_STREAM_LONGOPT: {
                So opt = so_i0(carg, 2);
                Argx *argx = t_argx_get(&arg->t_opt, opt);
                if(!argx) {
                    Argx pseudo = { .opt = opt };
                    arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_OPTION_ROOT, &pseudo);
                    status = -1;
                    goto error_but_maybe_get_env_help;
                }
                if(arg_parse_option(arg, stream, argx)) {
                    arg_parse_error(arg, stream, ARG_PARSE_ERROR_UNHANDLED_POSITIONAL, argx);
                    status = -1;
                    goto error_but_maybe_get_env_help;
                }
            } break;
            case ARG_STREAM_SHORTOPT: {
                So opts = so_i0(carg, 1);
                for(size_t i = 0; i < so_len(opts); ++i) {
                    unsigned char c = so_at(opts, i);
                    Argx *argx = 0;
                    if(stream->rest) {
                        vso_push(stream->rest->val.vso, so_i0(opts, i));
                        break;
                    } else {
                        if(c >= ARGX_SHORT_MIN && c < ARGX_SHORT_MAX) {
                            argx = arg->c[c - ARGX_SHORT_MIN];
                        }
                    }
                    if(!argx) {
                        Argx pseudo = { .opt = so_ll(&c, 1) };
                        arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_OPTION_ROOT, &pseudo);
                        status = -1;
                        goto error_but_maybe_get_env_help;
                    }
                    if(arg_parse_option(arg, stream, argx)) {
                        arg_parse_error(arg, stream, ARG_PARSE_ERROR_UNHANDLED_POSITIONAL, argx);
                        status = -1;
                        goto error_but_maybe_get_env_help;
                    }
                }
                if(!so_len(opts)) {
                    Argx pseudo = { .opt = carg };
                    arg_parse_error(arg, stream, ARG_PARSE_ERROR_MISSING_SHORTOPT, &pseudo);
                }
            } break;
        }
error_but_maybe_get_env_help:
        //if(!status) continue;
        if(arg->help.wanted) stream->skip_flag_check = true;
        if(status) break;
#if 0
        if(stream->is_help_lookup) {
            if(get_env_help) {
                Argx *argx = arg->help.last;
                if(argx->id == ARGX_TYPE_GROUP) {
                    ASSERT_ARG(argx->group_s);
                    argx = t_argx_get(argx->group_s->table, carg);
                    if(argx) {
                        arg_parse_set_help_any(arg, argx);
                        get_env_help = true;
                    } else {
                        arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_OPTION_GROUP, arg->help.last);
                        status = -1;
                    }
                } else {
                    arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_OPTION_GROUP, argx);
                    status = -1;
                }
            } else {
                Argx *argx_env = t_argx_get(&arg->t_env, carg);
                if(argx_env) {
                    arg_parse_set_help_any(arg, argx_env);
                    get_env_help = true;
                }
                Argx *argx_pos = t_argx_get(&arg->t_pos, carg);
                if(argx_pos) {
                    arg_parse_set_help_any(arg, argx_pos);
                    get_env_help = true;
                }
                if(!argx_env && !argx_pos && !arg->help.last) {
                    Argx pseudo = { .opt = carg };
                    arg_parse_error(arg, stream, ARG_PARSE_ERROR_INVALID_OPTION_ROOT, &pseudo);
                    status = -1;
                }
            }
        } else {
            break;
        }
        //if(!get_env_help) {
        //    break;
        //}
#endif
    }


    return status;
}

/* main parsing section }}} */

/* set reference value {{{ */

void arg_parse_setref_sources_mono(Argx *argx, So src, size_t n) {
    for(size_t i = 0; i < n; ++i) {
        Arg_Stream_Source source = { .path = src };
        arg_parse_add_source(argx, source);
    }
}

void arg_parse_setref_argx(Argx *argx) {
    if(argx->sources) return; /* do not setref if it was already parsed somewhere else */
    if(argx->ref.any) {
        if(argx->attr.is_array) {
            switch(argx->id) {
                default: ABORT(ERR_UNREACHABLE("unhandled id %u"), argx->id);
                case ARGX_TYPE_BOOL: {
                    if(argx->val.vb) array_extend(*argx->val.vb, *argx->ref.vb);
                } break;
                case ARGX_TYPE_COLOR: {
                    if(argx->val.vc) array_extend(*argx->val.vc, *argx->ref.vc);
                } break;
                case ARGX_TYPE_URI:
                case ARGX_TYPE_STRING: {
                    if(argx->val.vso) array_extend(*argx->val.vso, *argx->ref.vso);
                } break;
                case ARGX_TYPE_INT: {
                    if(argx->val.vi) array_extend(*argx->val.vi, *argx->ref.vi);
                } break;
                case ARGX_TYPE_SIZE: {
                    if(argx->val.vz) array_extend(*argx->val.vz, *argx->ref.vz);
                } break;
                case ARGX_TYPE_NONE: break;
                case ARGX_TYPE_REST: ABORT(ERR_UNREACHABLE("case will never provide default values"));
                case ARGX_TYPE_GROUP: ABORT(ERR_UNREACHABLE("case is handled separately"));
                case ARGX_TYPE_ENUM: ABORT(ERR_UNREACHABLE("case is handled separately"));
                case ARGX_TYPE_FLAG: ABORT(ERR_UNREACHABLE("case is handled separately"));
            }
            arg_parse_setref_sources_mono(argx, ARGX_SOURCE_REFVAL, array_len(*argx->ref.vb));
        } else {
            //printff(" v %p = r %p id %u [%.*s]",argx->val,argx->ref,argx->id,SO_F(argx->opt));
            arg_parse_setref_sources_mono(argx, ARGX_SOURCE_REFVAL, (int)(bool)(argx->ref.any));
            switch(argx->id) {
                default: ABORT(ERR_UNREACHABLE("unhandled id %u"), argx->id);
                case ARGX_TYPE_FLAG: {
                    bool have_sources = argx_flag_is_any_source_set(argx);
                    //printff("FLAG->HAVE SOURCE %u",have_sources);
                    if(have_sources) break;
                    /* check parent sources as well */
                    ASSERT_ARG(argx->group_p);
                    Argx *parent = argx->group_p->parent;
                    ASSERT_ARG(parent);
                    //if(!parent->sources) {
                        if(argx->val.b) *argx->val.b = *argx->ref.b;
                    //}
                } break;
                case ARGX_TYPE_COLOR: {
                    if(argx->val.c) *argx->val.c = *argx->ref.c;
                } break;
                case ARGX_TYPE_BOOL: {
                    if(argx->val.b) *argx->val.b = *argx->ref.b;
                } break;
                case ARGX_TYPE_URI:
                case ARGX_TYPE_STRING: {
                    if(argx->val.so) *argx->val.so = *argx->ref.so;
                } break;
                case ARGX_TYPE_INT: {
                    if(argx->val.i) *argx->val.i = *argx->ref.i;
                } break;
                case ARGX_TYPE_SIZE: {
                    if(argx->val.z) *argx->val.z = *argx->ref.z;
                } break;
                case ARGX_TYPE_NONE: break;
                case ARGX_TYPE_REST: ABORT(ERR_UNREACHABLE("case will never provide default values"));
                case ARGX_TYPE_GROUP: ABORT(ERR_UNREACHABLE("case is handled separately"));
                case ARGX_TYPE_ENUM: ABORT(ERR_UNREACHABLE("case is handled separately"));
            }
        }
    } else {
        argx->val.any = 0;
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
    for(Argx_Group **group = arg->opts; group < array_itE(arg->opts); ++group) {
        arg_parse_setref_group(*group);
    }
    arg_parse_setref_group(&arg->env);
}

/* set reference value }}} */

/* parsing entry points {{{ */

int arg_parse_environment(struct Arg *arg) {
    /* gather environment variables */
    int status = 0;
    Argx **itE = array_itE(arg->env.list);
    So env = SO, carg = SO;
    Arg_Stream stream_env = {
        .source.path = ARGX_SOURCE_ENVVARS,
    };
    for(Argx **it = arg->env.list; it < itE && !status; ++it) {
        so_env_get(&env, (*it)->opt);
        //printff("PARSE ENV: %.*s: [%.*s]", SO_F((*it)->opt), SO_F(env));
        if(!so_len(env)) continue;
        arg_stream_clear(&stream_env);
        vso_push(&stream_env.vso, env);
        arg_stream_get_next(&stream_env, &carg, &arg->builtin.compgen_flags);
        status = arg_parse_argx(arg, &stream_env, *it, carg);
    }
    arg_stream_free(&stream_env);
    return status;
}

int arg_parse_stdin(struct Arg *arg, const int argc, const char **argv) {
    int status = 0;
    /* parse stdin */
    arg->stream_in.source.path = ARGX_SOURCE_STDIN,
    arg_stream_from_stdin(&arg->stream_in, argc, argv);
    status = arg_parse_stream(arg, &arg->stream_in);
    /* check if there are missing positional values */
    if(arg->i_pos < array_len(arg->pos.list)) {
        //arg_parse_errmsg_missing_positionals(arg);
        arg_parse_error(arg, &arg->stream_in, ARG_PARSE_ERROR_MISSING_POSITIONAL, array_at(arg->pos.list, arg->i_pos));
        status = -1;
    }
    arg_stream_free(&arg->stream_in);
    return status;
}

int arg_queue_post_parsing(Arg *arg) {
    int result = 0;
    Argx_Callback_Queue *itE = array_itE(arg->queue);
    for(Argx_Callback_Queue *it = arg->queue; it < itE; ++it) {
        ASSERT_ARG(it->argx);
        ASSERT_ARG(it->argx->callback.func);
        result = it->argx->callback.func(it->argx, it->argx->callback.user, it->so);
        if(arg->builtin.quit_early) break;
        if(result) break;
    }
    return result;
}

void arg_parse_help_fmt_rec(So *out, Argx *argx) {
    if(!argx) return;
    arg_parse_help_fmt_rec(out, argx->group_p ? argx->group_p->parent : 0);
    argx_fmt_help(out, argx);
}

void arg_parse_help(Arg *arg) {
    Argx *help = arg->help.wanted ? arg->help.last : arg->help.error;

    size_t help_len = array_len(arg->help.sub);
    bool help_compgen = (arg->help.wanted && help == arg->help.argx && arg->builtin.compgen);
    bool help_do = (help_len || help_compgen) && !arg->builtin.config_print_selected;

    if(help_do) {
        if(help_compgen) {
            arg->builtin.nocolor = true;
        }
        Arg_Stream stream_help = {
            .source = { .path = so("help") },
            .is_help_lookup = true,
        };

        for(size_t i = 0; i < help_len || help_compgen; ++i) {
            help_compgen = false;
            So search = i < help_len ? array_at(arg->help.sub, i) : SO;
            stream_help.error_id = 0;
            Argx_Group *group = 0;
            Argx *help = arg_parse_hierarchy(arg, &stream_help, search, &group);
            if(help) {
                if(arg->builtin.compgen) {
                    arg_compgen_help_argx(arg, help);
                } else {
                    arg_help_argx(help);
                }
            } else if(group) {
                if(arg->builtin.compgen) {
                    arg_compgen_help_group(arg, group);
                } else {
                    arg_help_argx_group(group);
                }
            } else {
                if(arg->builtin.compgen) {
                    arg_compgen_help_groups(arg);
                } else {
                    Argx pseudo = { .opt = search };
                    arg_parse_error(arg, &stream_help, ARG_PARSE_ERROR_HIERARCHY_OPTION_CONFIG, &pseudo);
                }
                continue;
            }
        }

    } else {

        if(!help) {
            if(arg->builtin.compgen) {
                arg_compgen_global(arg);
            } else if(arg->builtin.config_print_selected) {
                arg_config(arg);
            }
        } else if(help == arg->help.argx) {
            if(arg->builtin.compgen) {
                arg_compgen_global(arg); /* TODO never reached */
            } else {
                arg_help(arg);
            }
        } else {
            if(arg->builtin.compgen) {
                arg_compgen_argx(arg, help);
            } else {
                arg_help_argx(help);
            }
        }
    }

}

void arg_parse_configs(Arg *arg) {
    ASSERT_ARG(arg);
    So extend = SO;
    for(size_t i = 0; i < array_len(arg->builtin.sources_vso); ++i) {
        So path = array_at(arg->builtin.sources_vso, i);
        so_clear(&extend);
        so_extend_wordexp(&extend, path, false);
        so_path_get_realpath(&extend, extend);
        //printff("SOURCE [%.*s]",SO_F(extend));
        /* check if I already loaded a file at that location */
        bool have_already_loaded = false;
        for(size_t j = 0; j < array_len(arg->builtin.sources_paths); ++j) {
            So loaded = array_at(arg->builtin.sources_paths, j);
            if(so_cmp(loaded, extend)) continue;
            have_already_loaded = true;
            break;
        }
        if(have_already_loaded) {
            //printff("ALREADY LOADED");
            continue;
        }
        /* can safely load the file for the first time */
        So content = SO;
        if(!so_file_read(extend, &content)) {
            vso_push(&arg->builtin.sources_paths, extend);
            vso_push(&arg->builtin.sources_content, content);
            //printff("PARSE CONFIG [%.*s]",SO_F(extend));
            arg_parse_config(arg, content, extend);
            so_zero(&extend);
        } else {
            //printff("COULD NOT OPEN [%.*s]",SO_F(extend));
        }
    }
    so_free(&extend);
    arg->help.error = 0;
    arg->help.last = 0;
}

/* these set the sources if there is NOT a refval present -- opposite of setref */
void arg_parse_setup_sources_argx(Argx *argx) {
    if(argx->sources) return; /* do not setref if it was already parsed somewhere else */
    if(!argx->ref.any) {
        if(argx->attr.is_array) {
            arg_parse_setref_sources_mono(argx, ARGX_SOURCE_REFVAL, array_len(*argx->val.vb));
        } else {
            arg_parse_setref_sources_mono(argx, ARGX_SOURCE_REFVAL, 1);
        }
    }
}

/* these set the sources if there is NOT a refval present -- opposite of setref */
void arg_parse_setup_sources_group(Argx_Group *group) {
    if(!group) return;
    for(Argx **it = group->list; it < array_itE(group->list); ++it) {
        //printff("setref for: %.*s",SO_F((*it)->opt));
        ASSERT_ARG(it);
        Argx *argx = *it;
        ASSERT_ARG(argx);
        if(argx->id == ARGX_TYPE_GROUP) {
            arg_parse_setup_sources_group(argx->group_s); 
        } else {
            arg_parse_setup_sources_argx(argx);
        }
    }
}

void arg_parse_enable_config_print(Arg *arg) {
    ASSERT_ARG(arg);
    if(!arg->builtin.config_use_builtin) return;
    argx_builtin_env_config(arg);
}

int arg_parse(struct Arg *arg, const int argc, const char **argv, bool *quit_early) {
    ASSERT_ARG(arg);
    ASSERT_ARG(quit_early);

    /* check if we want config generation support */
    arg_parse_enable_config_print(arg);

    if(!isatty(STDOUT_FILENO)) arg->builtin.nocolor = true;

    /* now actually parse */

    int status = 0;

    for(Argx_Group **it = arg->opts; it < array_itE(arg->opts); ++it) {
        arg_parse_setup_sources_group(*it);
    }
    arg_parse_setup_sources_group(&arg->env);
    arg_parse_setup_sources_group(&arg->pos);

    if(!status) status = arg_parse_environment(arg);
    if(arg->builtin.config_print_selected) arg->builtin.nocolor = true;
    if(arg->builtin.quit_early) goto defer;

    arg_parse_configs(arg);

    if(!status) status = arg_parse_stdin(arg, argc, argv);
    if(arg->builtin.quit_early) goto defer;

    if(!status) status = arg_queue_post_parsing(arg);
    if(arg->builtin.quit_early) goto defer;

    arg_parse_setref(arg);

defer:

    arg_parse_help(arg);

    *quit_early = arg->builtin.quit_early || arg->builtin.quit_when_all_valid;
    return status;
}

/* parsing entry points }}} */

