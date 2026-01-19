#include "arg.h"

int arg_parse_stream(struct Arg *arg, const int argc, const char **argv) {
    /* set the stream */
    arg->stream.argc = argc;
    arg->stream.argv = argv;
    arg->stream.i = 0;
    /* now parse */
    return 0;
}

int arg_parse(struct Arg *arg, const int argc, const char **argv) {

    arg_parse_stream(arg, argc, argv);

    return 0;
}

