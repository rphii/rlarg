
#include "../rlarg.h"
#include "arg-stream.h"

bool arg_stream_souces_only_contains(Arg_Stream_Source *sources, So cmp) {
    Arg_Stream_Source *itE = array_itE(sources);
    bool result = false;
    for(Arg_Stream_Source *it = sources; it < itE; ++it) {
        result = (!it->line_number && !so_cmp(it->path, cmp));
    }
    return result;
}

void arg_stream_free(Arg_Stream *stream) {
    vso_free(&stream->vso);
    memset(stream, 0, sizeof(*stream));
}

void arg_stream_clear(Arg_Stream *stream) {
    vso_clear(&stream->vso);
    stream->i = 0;
    stream->i_split = 0;
    stream->skip_flag_check = false;
    stream->rest = 0;
    stream->error_id = 0;
    so_zero(&stream->carg);
}

void arg_stream_from_stdin(Arg_Stream *stream, const int argc, const char **argv) {
    ASSERT_ARG(stream);
    ASSERT_ARG(argv);
    ASSERT_ARG(argc);
    for(size_t i = 1; i < argc; ++i) {
        vso_push(&stream->vso, so_l((char *)argv[i]));
    }
}

bool arg_stream_get_next(Arg_Stream *stream, So *val, bool *compgen_flags) {
    ASSERT_ARG(stream);
    ASSERT_ARG(val);
    if(!arg_stream_advance(stream)) return false;
    So carg = array_at(stream->vso, stream->i);
    if(stream->i_split) {
        stream->carg = so_i0(carg, stream->i_split);
    } else {
        stream->carg = so_split_ch(carg, '=', 0);
    }
    if(!stream->skip_flag_check && !so_cmp(stream->carg, so("--"))) {
        if(compgen_flags) *compgen_flags = true;
        stream->skip_flag_check = true;
        return arg_stream_get_next(stream, val, compgen_flags);
    } else {
        if(compgen_flags) *compgen_flags = false;
    }
    *val = stream->carg;
    return true;
}

bool arg_stream_advance(Arg_Stream *stream) {
    size_t len = array_len(stream->vso);
    if(stream->carg.str && !stream->not_consumed && stream->i < len) {
        So carg = array_at(stream->vso, stream->i);
        if(stream->carg.str == carg.str) {
            if(stream->carg.len < carg.len) {
                /* probably a split on = */
                stream->i_split = stream->carg.len + 1;
            } else {
                /* full-length token found, e.g. --help */
                ++stream->i;
            }
        } else {
            /* other side of split, --val=THIS-SIDE */
            stream->i_split = 0;
            ++stream->i;
        }
    }
    //printff("i %u < len %zu / not consumed %u", stream->i, len, stream->not_consumed);
    stream->not_consumed = false;
    return (stream->i < len);
}

void arg_stream_not_consumed(Arg_Stream *stream) {
    stream->not_consumed = true;
}

