#ifndef RLARG_STREAM_H

#include <rlso.h>
#include <stddef.h>
#include "arg-parse.h"

/* consider:
 *  --switch=value
 *  --switch value
 *  switch=value
 *  switch value
 *  toml? (configuration file)
 */

typedef enum {
    ARG_STREAM_SOURCE_NONE,
    ARG_STREAM_SOURCE_STDIN,
    ARG_STREAM_SOURCE_REFVAL,
    ARG_STREAM_SOURCE_CONFIG,
    ARG_STREAM_SOURCE_ENVVARS,
    ARG_STREAM_SOURCE_HELP,
} Arg_Stream_Source_List;

typedef struct Arg_Stream_Source {
    So path;
    int number;
    size_t nb_source;
    Arg_Stream_Source_List id;
} Arg_Stream_Source;


typedef struct Arg_Stream {
    VSo vso;
    int i, i_split;
    bool skip_flag_check;   /* set true once we encounter '--' */
    bool not_consumed;
    bool is_config;
    bool is_help_lookup;
    struct Argx *rest;      /* pointer to an argx of type ARGX_TYPE_REST */
    So carg;
    Arg_Stream_Source source;
    Arg_Parse_Error_List error_id;
} Arg_Stream;

void arg_stream_free(Arg_Stream *stream);
void arg_stream_source_free(Arg_Stream_Source *source);
void arg_stream_clear(Arg_Stream *stream);

void arg_stream_from_stdin(Arg_Stream *stream, const int argc, const char **argv);

bool arg_stream_get_next(Arg_Stream *stream, So *val, bool *compgen_flags);
bool arg_stream_advance(Arg_Stream *stream);
void arg_stream_not_consumed(Arg_Stream *stream);

#define RLARG_STREAM_H
#endif /* RLARG_STREAM_H */

