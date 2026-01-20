#include "../rlarg.h"
#include <rlc.h>

int main(void) {
    struct Arg *arg = arg_new();
    ASSERT(arg, "expect to have pointer");

    struct Argx_Group *g1 = argx_group(arg, so("default"));
    struct Argx_Group *g2 = argx_group(arg, so("default"));

    printff("group 'default' pointers: %p, %p", g1, g2);
    ASSERT(g1 == g2, "should point to the same");

    struct Argx *x, *e1, *e2, *e3;

    x = argx(g1, 0, so("hi"), so("description"));
        printff("p %p", x);

    bool p = false;
    bool d = true;
    x = argx(g1, 'x', so("boolr"), so("a required boolean value"));
        printff("p %p", x);
        argx_type_bool(x, &p, &d);

    x = argx(g1, 'X', so("boolo"), so("a optional boolean value"));
        printff("p %p", x);
        argx_type_bool(x, &p, &d);
        argx_hint_kind(x, ARGX_HINT_OPTIONAL);

    VSo vec_p = 0;
    VSo vec_v = 0;
    vso_push(&vec_v, so("one"));
    vso_push(&vec_v, so("two"));
    x = argx(g1, 'v', so("vso"), so("list of strings"));
        argx_type_array_so(x, &vec_p, &vec_v);

    int e_out;
    x=argx(g1, 'e', so("enumerator"), so("list of strings"));
      g2=argx_group_enum(x, &e_out, 0);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx(g1, 0, so("enumerator2"), so("list of strings"));
      g2=argx_group_enum(x, &e_out, 0);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx(g1, 0, so("enumerator33"), so("list of strings"));
      g2=argx_group_enum(x, &e_out, 0);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx(g1, 0, so("enumerator444"), so("list of strings"));
      g2=argx_group_enum(x, &e_out, 0);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx(g1, '1', so("1111"), so("nothing"));
    x=argx(g1, '2', so("somerandom"), so("nothing"));
// => for flags:
//           argx_enum_all(g2, so("all"), so("enable all"));
//        x =argx_enum_unique(g2, so("default"), so("default option"));
//           argx_enum_bind(x, e1);
//           argx_enum_bind(x, e2);

    arg_help(arg);
    //arg_config(arg);

    arg_free(&arg);
    vso_free(&vec_v);
    return 0;
}

