#include <rlso.h>
#include <rlc/array.h>
#include "../src/rlarg.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

typedef enum {
    CONFIG_NONE,
    CONFIG_PRINT,
    CONFIG_BROWSER,
    CONFIG_LMAO,
} ConfigList;

typedef enum {
    CONFIG_MODE_NONE,
    CONFIG_MODE_HELLO,
    CONFIG_MODE_INT,
    CONFIG_MODE_FLOAT,
    CONFIG_MODE_STRING,
    CONFIG_MODE_BOOL,
    CONFIG_MODE_STRINGS,
} ConfigModeList;

typedef struct Config {
    ssize_t whole;
    bool boolean;
    double number;
    So config;
    So string;
    VSo strings;
    ConfigList id;
    ConfigList id2;
    ConfigList id3;
    struct {
        ConfigModeList id;
        ssize_t z;
        So s;
        double f;
        bool b;
        VSo v;
    } mode;
    struct {
        bool safe;
        bool unsafe;
        bool other;
    } flags;
} Config;

#define TEST(msg)   do { \
        printf(F("=== %s ===", BOLD FG_BL_B) "\n", #msg); \
        arg_free(&arg); \
        TRYC(main_arg(&arg, argc, argv)); \
    } while(0)

#define TEST_ALL        0
#if TEST_ALL
#define TEST_EMPTY_LONG 1
#define TEST_TWIN_LONG  1
#define TEST_TWIN_SHORT 1
#endif

int hello_world(int *n) {
    printf("Hello, %i worlds!\n", *n);
    return 0;
}

#include <sys/ioctl.h>
#include <poll.h> // int poll(struct pollfd *fds, nfds_t nfds, int timeout);

#include <errno.h>
#include <unistd.h>

int main(const int argc, const char **argv) {

    int err = 0;
    size_t n_arg = 0;
    Config config = {0};
    Config preset = {0};
    So *rest2 = {0};
    So *files = {0};
    So configuration = SO;
    struct Arg *arg = arg_new();
    struct ArgX *x;
    struct ArgXGroup *g;
    ssize_t nfuck = 0;
    bool quit_early = false;

    preset.flags.other = true;
    preset.flags.safe = true;
    preset.id = CONFIG_LMAO;
    preset.config = so("path/to/config/that-is-very-long-and-unnecessary");

    /* set up arguments {{{*/

    arg_init(arg, so_l(argv[0]), so("this is a test program to verify the functionality of an argument parser. also, this is a very very long and boring description, just so I can check whether or not it wraps and ends correctly! isn't that fascinating..."), so("github: " F("https://github.com/rphii", FG_BL_B UL)));
    arg_init_rest(arg, so("files"), &files);
    arg_init_width(arg, 40, 45);
    arg_init_fmt(arg);
    //arg_init_width(arg, 0, 45);

    struct ArgXGroup *opt = argx_group(arg, so("Options"), false);
    argx_builtin_opt_help(opt);
    argx_builtin_opt_source(opt, so("$HOME/.config/rphiic/colors.conf"));
    x=argx_init(opt, 0, so("xyz"), so("nothing"));
    x=argx_init(opt, 'b', so("bool"), so("boolean value and a long description that is"));
      argx_bool(x, &config.boolean, &preset.boolean);
    x=argx_init(opt, 'f', so("double"), so("double value"));
      argx_dbl(x, &config.number, &preset.number);
    x=argx_init(opt, 's', so("string"), so("string value"));
      argx_str(x, &config.string, &preset.string);
    x=argx_init(opt, 'i', so("integer"), so("integer value"));
      argx_int(x, (int *)&config.whole, (int *)&preset.whole);
    x=argx_init(opt, 'o', so("option"), so("select one option"));
      g=argx_opt(x, (int *)&config.id, (int *)&preset.id);
      //g=argx_opt(x, (int *)&config.id, 0);
        //x=argx_init(g, 0, so("none"), so("do nothing"));
        //  argx_opt_enum(x, CONFIG_NONE);
        x=argx_init(g, 0, so("none"), so("do nothing"));
          argx_opt_enum(x, CONFIG_NONE);
        x=argx_init(g, 0, so("print"), so("print stuff"));
          argx_opt_enum(x, CONFIG_PRINT);
        x=argx_init(g, 0, so("browser"), so("browse stuff"));
          argx_opt_enum(x, CONFIG_BROWSER);
        x=argx_init(g, 0, so("lmao"), so("what the fuck"));
          argx_opt_enum(x, CONFIG_LMAO);
          argx_ssz(x, &nfuck, 0);
          argx_func(x, ++n_arg, hello_world, &nfuck, false, false);
        x=argx_init(g, 0, so("test"), so("what the fuck"));
          argx_opt_enum(x, 6);
        x=argx_init(g, 0, so("useless"), so("what the fuck"));
          argx_opt_enum(x, 7);
        x=argx_init(g, 0, so("verbose"), so("what the fuck"));
          argx_opt_enum(x, 8);
    //x=argx_init(arg_opt(arg), n_arg++, 0, so("very-long-option-that-is-very-important-and-cool-but-serves-no-purpose-whatsoever-anyways-how-are-you-doing-today"), so("select another option"));
    x=argx_init(opt, 'F', so("flags"), so("set different flags"));
      g=argx_flag(x);
        x=argx_init(g, 0, so("safe"), so("enable safe operation"));
          argx_flag_set(x, &config.flags.safe, &preset.flags.safe);
        x=argx_init(g, 0, so("unsafe"), so("enable unsafe operation"));
          argx_flag_set(x, &config.flags.unsafe, &preset.flags.unsafe);
        x=argx_init(g, 0, so("other"), so("enable other operation"));
          argx_flag_set(x, &config.flags.other, &preset.flags.other);

    x=argx_pos(arg, so("mode"), so("the main mode"));
      g=argx_opt(x, (int *)&config.mode.id, (int *)&preset.mode.id);
        x=argx_init(g, 0, so("none"), so("do nothing"));
          argx_opt_enum(x, CONFIG_MODE_NONE);
        x=argx_init(g, 0, so("hello"), so("print hello"));
          argx_func(x, ++n_arg, hello_world, &nfuck, false, true);
          argx_opt_enum(x, CONFIG_MODE_HELLO);
        x=argx_init(g, 0, so("int"), so("set int"));
          argx_ssz(x, &config.mode.z, &preset.mode.z);
          argx_opt_enum(x, CONFIG_MODE_INT);
        x=argx_init(g, 0, so("float"), so("set float"));
          argx_dbl(x, &config.mode.f, &preset.mode.f);
          argx_opt_enum(x, CONFIG_MODE_FLOAT);
        x=argx_init(g, 0, so("string"), so("set string"));
          argx_str(x, &config.mode.s, &preset.mode.s);
          argx_opt_enum(x, CONFIG_MODE_STRING);
        x=argx_init(g, 0, so("bool"), so("set bool"));
          argx_bool(x, &config.mode.b, &preset.mode.b);
          argx_bool_require_tf(x, true);
          argx_opt_enum(x, CONFIG_MODE_BOOL);
        x=argx_init(g, 0, so("strings"), so("set strings"));
          argx_vstr(x, &rest2, 0);
          argx_opt_enum(x, CONFIG_MODE_STRINGS);

    x=argx_init(opt, 'I', so("input"), so("input files"));
      argx_vstr(x, &config.strings, &preset.strings);
      argx_type(x, so("input-files"));

    struct ArgXGroup *env = argx_group(arg, so("Environment Variables"), false);
    argx_builtin_env_compgen(env);
    x=argx_env(env, so("ARG_CONFIG_PATH"), so("config path"), false);
      argx_str(x, &config.config, &preset.config);

    struct ArgXGroup *rice = argx_group(arg, so("Color Adjustments"), true);
    argx_builtin_opt_rice(rice);

    /*}}}*/

#if 1
    /* load config {{{ */
    So filename = so("test_arg.conf");
    TRYC(so_file_read(filename, &configuration));
    arg_config(arg, so_ll(configuration.str, so_len(configuration)));
    /*}}}*/
#endif

#if 0
    sleep(1);
    So input = {0};
    int n = 0;
    while (ioctl(0, FIONREAD, &n) ? 0 : n > 0) {
    //while(true) {
        str_clear(&input);
        str_get_str(&input);
        if(!str_length(input)) continue;
        printff(F(">>> %.*s", BG_YL_B FG_BK), SO_F(input));
    }
    return 0;
#endif

    TRYC(arg_parse(arg, argc, argv, &quit_early));
    if(quit_early) goto clean;


    /* post arg parse {{{ */
    printf("quit? %s\n", quit_early ? "yes" : "no");
    printf("INPUT-FILES:\n");
    for(size_t i = 0; i < array_len(config.strings); ++i) {
        printf(" %.*s\n", SO_F(array_at(config.strings, i)));
    }
    printf("FILES:\n");
    for(size_t i = 0; i < array_len(files); ++i) {
        printf(" %.*s\n", SO_F(array_at(files, i)));
    }
    printf("STRINGS:\n");
    for(size_t i = 0; i < array_len(rest2); ++i) {
        printf(" %.*s\n", SO_F(array_at(rest2, i)));
    }
    /*}}}*/

clean:
    so_free(&configuration);
    arg_free(&arg);
    return err;

error:
    ERR_CLEAN;
}

