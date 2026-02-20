#include "../rlarg/arg-parse-config.h"

int main(int argc, char **argv) {
    int result = -1;
    Arg *arg = arg_new();

    if(argc != 3) goto defer;

    So expect_so = so_l(argv[1]);
    So path = so_l(argv[2]);

    int expect = 0;
    if(so_as_int(expect_so, &expect, 0)) goto defer;
    So content = SO;
    if(so_file_read(path, &content)) goto defer;

    result = arg_parse_config(arg, content, path);
    if(result == expect) result = 0;

defer:
    so_free(&content);
    arg_free(&arg);

    printf("status: %u\n",result);

    return result;
}


