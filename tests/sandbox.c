#include "../rlarg.h"

/* options 
 TEST_INT
 TEST_VINT
 TEST_SIZE
 TEST_VSIZE
 TEST_BOOL
 TEST_VBOOL
 TEST_SO
 TEST_VSO
 TEST_COLOR
 TEST_VCOLOR
*/

#ifndef TEST_INT
#define TEST_INT    1
#endif
#ifndef TEST_VINT
#define TEST_VINT   1
#endif
#ifndef TEST_SIZE
#define TEST_SIZE   1
#endif
#ifndef TEST_VSIZE
#define TEST_VSIZE   1
#endif
#ifndef TEST_BOOL
#define TEST_BOOL   1
#endif
#ifndef TEST_VBOOL
#define TEST_VBOOL   1
#endif
#ifndef TEST_COLOR
#define TEST_COLOR   1
#endif
#ifndef TEST_VCOLOR
#define TEST_VCOLOR   1
#endif
#ifndef TEST_SO
#define TEST_SO   1
#endif
#ifndef TEST_VSO
#define TEST_VSO   1
#endif

typedef struct Sandbox {

#if TEST_INT
    int i;
#endif

#if TEST_VINT
    int *vi;
#endif

#if TEST_SIZE
    ssize_t z;
#endif

#if TEST_VSIZE
    ssize_t *vz;
#endif

#if TEST_BOOL
    bool b;
#endif

#if TEST_VBOOL
    bool *vb;
#endif

#if TEST_SO
    So s;
#endif

#if TEST_VSO
    VSo vs;
#endif

#if TEST_COLOR
    Color c;
#endif

#if TEST_VCOLOR
    Color *vc;
#endif

} Sandbox;

int main(int argc, const char **argv) {
    Sandbox r = {0};

    /* set up arg */
    struct Arg *arg = arg_new();
    struct Argx_Group *g = argx_group(arg, so("default"));
    struct Argx *x;

    argx_builtin_env_nocolor(arg);
    argx_builtin_opt_help(g);
    arg_enable_config_print(arg, true);
    argx_builtin_opt_source(g, so("readme.conf"));

    /* add options */

#if TEST_INT
    x=argx_opt(g, 0, so("int"), so("desc"));
      argx_type_int(x, &r.i, 0);
#endif

#if TEST_VINT
    x=argx_opt(g, 0, so("vint"), so("desc"));
      argx_type_array_int(x, &r.vi, 0);
#endif

#if TEST_SIZE
    x=argx_opt(g, 0, so("size"), so("desc"));
      argx_type_size(x, &r.z, 0);
#endif

#if TEST_VSIZE
    x=argx_opt(g, 0, so("vsize"), so("desc"));
      argx_type_array_size(x, &r.vz, 0);
#endif

#if TEST_BOOL
    x=argx_opt(g, 0, so("bool"), so("desc"));
      argx_type_bool(x, &r.b, 0);
#endif

#if TEST_VBOOL
    x=argx_opt(g, 0, so("vbool"), so("desc"));
      argx_type_array_bool(x, &r.vb, 0);
#endif

#if TEST_SO
    x=argx_opt(g, 0, so("so"), so("desc"));
      argx_type_so(x, &r.s, 0);
#endif

#if TEST_VSO
    x=argx_opt(g, 0, so("vso"), so("desc"));
      argx_type_array_so(x, &r.vs, 0);
#endif

#if TEST_COLOR
    x=argx_opt(g, 0, so("color"), so("desc"));
      argx_type_color(x, &r.c, 0);
#endif

#if TEST_VCOLOR
    x=argx_opt(g, 0, so("vcolor"), so("desc"));
      argx_type_array_color(x, &r.vc, 0);
#endif


    /* parse */
    bool quit_early = false;
    int result = 0;
    result = arg_parse(arg, argc, argv, &quit_early);
    if(result || quit_early) goto defer;

    /* main code */
    printf("Hello, README! The int is: %i\n", r.i);

defer:
    arg_free(&arg);
    return result;
}

