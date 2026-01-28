
#include "../rlarg.h"
#include "arg-stream.h"

bool arg_stream_get_next(Arg_Stream *stream, So *val) {
    ASSERT_ARG(stream);
    ASSERT_ARG(val);
    if(!arg_stream_advance(stream)) return false;
    So carg = so_l((char *)stream->argv[stream->i]);
    if(stream->i_split) {
        stream->carg = so_i0(carg, stream->i_split);
    } else {
        stream->carg = so_split_ch(carg, '=', 0);
    }
    if(!stream->skip_flag_check && !so_cmp(stream->carg, so("--"))) {
        stream->skip_flag_check = true;
        return arg_stream_get_next(stream, val);
    }
    *val = stream->carg;
    return true;
}

bool arg_stream_advance(Arg_Stream *stream) {
    if(stream->carg.str && !stream->not_consumed) {
        So carg = so_l((char *)stream->argv[stream->i]);
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
    stream->not_consumed = false;
    return (stream->i < stream->argc);
}

void arg_stream_not_consumed(Arg_Stream *stream) {
    stream->not_consumed = true;
}

