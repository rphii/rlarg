#ifndef ARG_PARSE_H

#include <rlso.h>

typedef enum Arg_Parse_Error_List {
    ARG_PARSE_ERROR_NONE,
    ARG_PARSE_ERROR_NO_REST_ALLOWED,
    ARG_PARSE_ERROR_UNHANDLED_POSITIONAL,
    ARG_PARSE_ERROR_MISSING_POSITIONAL,
    ARG_PARSE_ERROR_MISSING_VALUE,
    ARG_PARSE_ERROR_MISSING_SHORTOPT,
    ARG_PARSE_ERROR_MISSING_FILE_DELIM,
    ARG_PARSE_ERROR_MISSING_ARRAY_DELIM,
    ARG_PARSE_ERROR_MISSING_HIERARCHY_DELIM,
    ARG_PARSE_ERROR_MISSING_STRING_DELIM,
    ARG_PARSE_ERROR_INVALID_CONVERSION,
    ARG_PARSE_ERROR_INVALID_OPTION_GROUP,
    ARG_PARSE_ERROR_INVALID_OPTION_ROOT,
    ARG_PARSE_ERROR_INVALID_STRING,
    ARG_PARSE_ERROR_INVALID_STRING_END,
    ARG_PARSE_ERROR_INVALID_FILE,
    ARG_PARSE_ERROR_INVALID_SECTION,
    ARG_PARSE_ERROR_HIERARCHY_OPTION_CONFIG,
    ARG_PARSE_ERROR_HIERARCHY_TABLE_CONFIG,
    ARG_PARSE_ERROR_HIERARCHY_ROOT_CONFIG,
    ARG_PARSE_ERROR_UNCONFIGURABLE,
} Arg_Parse_Error_List;

struct Arg;
struct Arg_Stream;
struct Argx;
struct Argx_Group;

int arg_parse_argx(struct Arg *arg, struct Arg_Stream *stream, struct Argx *argx, So so);
struct Argx *arg_parse_hierarchy(struct Arg *arg, struct Arg_Stream *stream, So lhs, struct Argx_Group **root_group);
void arg_parse_error(struct Arg *arg, struct Arg_Stream *stream, Arg_Parse_Error_List id, struct Argx *argx);

#define ARG_PARSE_H
#endif /* ARG_PARSE_H */

