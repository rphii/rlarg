#ifndef RLARG_PARSE_CONFIG_H

#include <rlso.h>
#include "arg.h"

typedef enum {
    ARG_PARSE_CONFIG_DEFAULT,
    ARG_PARSE_CONFIG_STRING,
    ARG_PARSE_CONFIG_FILE,
    ARG_PARSE_CONFIG_ARRAY,
} Arg_Parse_Config_List;

typedef struct Arg_Parse_Config {
    Arg *arg;
    So section;
    So hierarchy;
} Arg_Parse_Config;

int arg_parse_config(struct Arg *arg, So config, So path);


#define RLARG_PARSE_CONFIG_H
#endif /* RLARG_PARSE_CONFIG_H */

