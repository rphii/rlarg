#include "../rlarg.h"
#include <rlc.h>

int main(const int argc, const char **argv) {
    struct Arg *arg = arg_new();
    ASSERT(arg, "expect to have pointer");

    struct Argx_Group *g1 = argx_group(arg, so("default"));
    struct Argx_Group *g2 = argx_group(arg, so("default"));
    struct Argx_Group *g3 = 0;

    printff("group 'default' pointers: %p, %p", g1, g2);
    ASSERT(g1 == g2, "should point to the same");

    struct Argx *x, *e1, *e2, *e3;

    x = argx_opt(g1, 0, so("hi"), so("description"));
        printff("p %p", x);

    bool p = false;
    bool d = true;
    x = argx_opt(g1, 'x', so("boolr"), so("a required boolean value"));
        printff("p %p", x);
        argx_type_bool(x, &p, &d);

    x = argx_opt(g1, 'X', so("boolo"), so("a optional boolean value"));
        printff("p %p", x);
        argx_type_bool(x, &p, &d);
        argx_hint_kind(x, ARGX_HINT_OPTIONAL);

    bool f1, f2, f3;
    VSo vec_p = 0;
    VSo vec_v = 0;
    vso_push(&vec_v, so("one"));
    vso_push(&vec_v, so("two"));
    x = argx_opt(g1, 'v', so("vso"), so("list of strings"));
        argx_type_array_so(x, &vec_p, &vec_v);

    int e_out, e_default = 1;
    x=argx_opt(g1, 'e', so("enumerator"), so("list of strings"));
      g2=argx_group_enum(x, &e_out, &e_default);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx_opt(g1, 0, so("enumerator2"), so("list of strings"));
      g2=argx_group_enum(x, &e_out, 0);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx_opt(g1, 0, so("enumerator33"), so("list of strings"));
      g2=argx_group_enum(x, &e_out, 0);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx_opt(g1, 0, so("enumerator444"), so("list of strings"));
      g2=argx_group_enum(x, &e_out, 0);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx_opt(g1, 0, so("flag"), so("some flags"));
      g2=argx_group_flags(x);
        e1=argx_flag(g2, &f1, &(bool){true}, so("sfw"), so("safe for work"));
        e2=argx_flag(g2, &f2, &(bool){false}, so("nsfw"), so("not safe for work"));
        e2=argx_flag(g2, &f3, &(bool){true}, so("sketchy"), so("risky for work"));

    So subopt = SO, subopt2 = SO;
    x=argx_opt(g1, 0, so("subopt4"), so("some options"));
      g2=argx_group_options(x);
        x=argx_opt(g2, 0, so("subopt3"), so("second subopt"));
          g3=argx_group_options(x);
            x=argx_opt(g3, 0, so("subopt"), so("final subopt"));
              argx_type_so(x, &subopt, 0);
            x=argx_opt(g3, 0, so("subopt2"), so("final subopt2"));
              argx_type_so(x, &subopt2, 0);
        x=argx_opt(g2, 0, so("subopt2"), so(""));

    x=argx_opt(g1, '1', so("1111"), so("nothing"));
    x=argx_opt(g1, '2', so("somerandom"), so("nothing"));

    x=argx_pos(arg, so("kind"), so("kind of thing to do"));
    x=argx_pos(arg, so("id"), so("id of thing"));
// => for flags:
//           argx_enum_all(g2, so("all"), so("enable all"));
//        x =argx_enum_unique(g2, so("default"), so("default option"));
//           argx_enum_bind(x, e1);
//           argx_enum_bind(x, e2);

    arg_parse(arg, argc, argv);
    arg_help(arg);
    printff("::::::CONFIG:::::");
    arg_config(arg);

    arg_free(&arg);
    vso_free(&vec_v);
    return 0;
}

