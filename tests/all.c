#include "../rlarg.h"
#include <rlc.h>

int main(void) {
    struct Arg *arg = arg_new();
    ASSERT(arg, "expect to have pointer");

    struct Argx_Group *g1 = argx_group(arg, so("default"));
    struct Argx_Group *g2 = argx_group(arg, so("default"));

    printff("group 'default' pointers: %p, %p", g1, g2);
    ASSERT(g1 == g2, "should point to the same");

    void *p1 = argx(g1, 0, so("hi"), so("description"));
    printff("p %p", p1);

    void *p2 = argx(g1, 'x', so("hii"), so("other description"));
    printff("p %p", p1);

    bool p = false;
    bool d = true;
    argx_type_bool(p2, &p, &d);
    arg_help(arg);

    return 0;
}

