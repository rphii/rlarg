#ifndef ARG_PARSE_H

#include <rlso.h>

typedef enum Arg_Parse_Error_List {
    ARG_PARSE_ERROR_NONE,
    ARG_PARSE_ERROR_NO_REST_ALLOWED,
    ARG_PARSE_ERROR_UNHANDLED_POSITIONAL,
    ARG_PARSE_ERROR_MISSING_POSITIONAL,
    ARG_PARSE_ERROR_MISSING_VALUE,
    ARG_PARSE_ERROR_MISSING_SHORTOPT,
    ARG_PARSE_ERROR_INVALID_CONVERSION,
    ARG_PARSE_ERROR_INVALID_OPTION_GROUP,
    ARG_PARSE_ERROR_INVALID_OPTION_ROOT,
} Arg_Parse_Error_List;

struct Arg;
struct Arg_Stream;
struct Argx;

int arg_parse_argx(struct Arg *arg, struct Arg_Stream *stream, struct Argx *argx, So so);

#define ARG_PARSE_H
#endif /* ARG_PARSE_H */

