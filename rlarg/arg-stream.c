
#include "../rlarg.h"
#include "arg-stream.h"

int arg_stream_get_next(Arg_Stream *stream, So *val) {
    ASSERT_ARG(stream);
    ASSERT_ARG(val);
    if(stream->i >= stream->argc) {
        return -1;
    }
    So carg = so_l(stream->argv[stream->i]);
    if(stream->i_split) {
        *val = so_i0(carg, stream->i_split);
    } else {
        *val = so_split_ch(carg, '=', 0);
    }
    if(!stream->skip_flag_check && !so_cmp(*val, so("--"))) {
        stream->skip_flag_check = true;
        arg_stream_advance(stream, *val);
        return arg_stream_get_next(stream, val);
    }
    return 0;
}

bool arg_stream_advance(Arg_Stream *stream, So gotten_next) {
    if(!gotten_next.str) return true;
    So carg = so_l(stream->argv[stream->i]);
    if(gotten_next.str == carg.str) {
        if(gotten_next.len < carg.len) {
            /* probably a split on = */
            stream->i_split = gotten_next.len + 1;
        } else {
            /* full-length token found, e.g. --help */
            ++stream->i;
        }
    } else {
        /* other side of split, --val=THIS-SIDE */
        stream->i_split = 0;
        ++stream->i;
    }
    return (stream->i < stream->argc);
}

