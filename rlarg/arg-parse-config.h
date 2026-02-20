#ifndef RLARG_PARSE_CONFIG_H

#include <rlso.h>
#include "arg.h"

typedef enum {
    ARG_PARSE_CONFIG_DEFAULT,
    ARG_PARSE_CONFIG_STRING,
    ARG_PARSE_CONFIG_FILE,
    ARG_PARSE_CONFIG_ARRAY,
} Arg_Parse_Config_List;

typedef enum {
    ARG_PARSE_CONFIG_OK         = 0,
    ARG_PARSE_CONFIG_ERR_CONFIG = (1L << 0), /* if any error has happened */
    ARG_PARSE_CONFIG_ERR_SYNTAX = (1L << 1), /* if syntax errors occures */
    ARG_PARSE_CONFIG_ERR_ASSIGN = (1L << 2), /* if assigning errors occured */
    ARG_PARSE_CONFIG_ERR_HIERAR = (1L << 3), /* if hierarchy errors occured */
    ARG_PARSE_CONFIG_ERR_FILE   = (1L << 4), /* if any file failed to read */
} Arg_Parse_Config_Flag;

typedef struct Arg_Parse_Config_Head {
    So so;
    size_t line_number;
} Arg_Parse_Config_Head;

typedef struct Arg_Parse_Config {
    Arg *arg;
    So section;
    So hierarchy;
    So file;
    /* other things */
    So tmp_full_hierarchy;
    So tmp_file_path;
    So tmp_string;
    So tmp_file_path_wordexp;
    Argx *argx;
    Arg_Stream stream;
    Arg_Parse_Config_Flag status;
} Arg_Parse_Config;

int arg_parse_config(struct Arg *arg, So config, So path);


#define RLARG_PARSE_CONFIG_H
#endif /* RLARG_PARSE_CONFIG_H */

