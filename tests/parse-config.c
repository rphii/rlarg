#include "../rlarg/arg-parse-config.h"

/* only check syntax */

int main(int argc, char **argv) {
    int result = -1;
    Arg *arg = arg_new();

    if(argc != 2) goto defer;

    So path = so_l(argv[1]);

    So content = SO;
    if(so_file_read(path, &content)) goto defer;

    result = arg_parse_config(arg, content, path);
    result &= ARG_PARSE_CONFIG_ERR_SYNTAX;

defer:
    so_free(&content);
    arg_free(&arg);

    printf("status: %u\n",result);

    return result;
}


