#include "../rlarg.h"
#include <rlc.h>

int cbtest(struct Argx *argx, void *user, So so) {
    printff(F("HELLO WORLD [%.*s]", FG_GN_B), SO_F(so));
    if(!so_cmp(so, so("111"))) {
        printff(F("QUITTING EARLY", FG_GN_B));
        arg_runtime_quit_early(argx, true);
    }
    return 0;
}

int main(const int argc, const char **argv) {
    struct Arg *arg = arg_new();
    ASSERT(arg, "expect to have pointer");
    arg_enable_config_print(arg, true);

    struct Argx_Group *g1 = argx_group(arg, so("default"));
    struct Argx_Group *g2 = argx_group(arg, so("default"));
    struct Argx_Group *g3 = 0;

    //printff("group 'default' pointers: %p, %p", g1, g2);
    ASSERT(g1 == g2, "should point to the same");

    argx_builtin_env_compgen(arg);
    argx_builtin_opt_color(g1);
    argx_builtin_opt_help(g1);
    //argx_builtin_opt_source(g1, so("$HOME/.config/rphiic/colors.conf"));
    argx_builtin_opt_source(g1, so("all.conf"));

    struct Argx *x, *e1, *e2, *e3, *xs, *xu, *xx;

    bool f1, f2, f3;
    VSo rest = 0;
    VSo vec_p = 0;
    VSo vec_v = 0;
#if 1
    x = argx_opt(g1, 0, so("hi"), so("description"));
        //printff("p %p", x);

    bool p1 = false, p2 = false;
    bool d = true;
    xs=argx_opt(g1, 0, so("switchtest"), so("set various things"));
       argx_type_switch(xs);

    x = argx_opt(g1, 'x', so("boolr"), so("a required boolean value"));
        //printff("p %p", x);
        argx_type_bool(x, &p1, &d);
        argx_switch_bool(xs, x, true);
    argx_attr_hide(x, true);

    x = argx_opt(g1, 'X', so("boolo"), so("a optional boolean value"));
        //printff("p %p", x);
        argx_type_bool(x, &p2, &d);
        argx_hint_kind(x, ARGX_HINT_OPTIONAL);

    So name = SO;
    x = argx_opt(g1, 'N', so("name"), so("a name"));
        argx_type_so(x, &name, 0);
        argx_switch_so(xs, x, so("nioh\""));

    int *vi = 0;

    vso_push(&vec_v, so("one"));
    vso_push(&vec_v, so("two"));
    x = argx_opt(g1, 'v', so("vso"), so("list of strings"));
        argx_type_array_so(x, &vec_p, &vec_v);
    x = argx_opt(g1, 'V', so("vi"), so("list of ints"));
        argx_type_array_int(x, &vi, 0);
        argx_switch_array_int(xs, x, vi);


    int e_out1, e_out2, e_out3, e_out4, e_default = 1;
    x=argx_opt(g1, 'e', so("enumerator"), so("list of strings"));
      g2=argx_group_enum(x, &e_out1, &e_default);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx_opt(g1, 0, so("enumerator2"), so("list of strings"));
      g2=argx_group_enum(x, &e_out2, 0);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx_opt(g1, 0, so("enumerator33"), so("list of strings"));
      g2=argx_group_enum(x, &e_out3, 0);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));

    x=argx_opt(g1, 0, so("enumerator444"), so("list of strings"));
      g2=argx_group_enum(x, &e_out4, 0);
        e1=argx_enum_bind(g2, 0, so("sfw"), so("safe for work"));
        e2=argx_enum_bind(g2, 1, so("nsfw"), so("not safe for work"));
        e2=argx_enum_bind(g2, 2, so("sketchy"), so("risky for work"));
#endif

    xs=argx_opt(g1, 0, so("flag-all"), so("set all flags"));
       argx_type_switch(xs);
    xu=argx_opt(g1, 0, so("flag-none"), so("unset all flags"));
       argx_type_switch(xu);

    x=argx_opt(g1, 0, so("flag"), so("some flags"));
      g2=argx_group_flags(x);
        e1=argx_flag(g2, &f1, &(bool){true}, so("sfw"), so("safe for work"));
           argx_switch_flag(xs, e1, true);
           argx_switch_flag(xu, e1, false);
        e2=argx_flag(g2, &f2, &(bool){false}, so("nsfw"), so("not safe for work"));
           argx_switch_flag(xs, e2, true);
           argx_switch_flag(xu, e2, false);
        e3=argx_flag(g2, &f3, &(bool){true}, so("sketchy"), so("risky for work"));
           argx_switch_flag(xs, e3, true);
           argx_switch_flag(xu, e3, false);


    int seqi = 0, seqe = 0;
    bool seqf0 = 0, seqf1 = 0, seqf2 = 0;
    So seqs = SO, seqs2 = SO;
    bool seqb = false;
    x=argx_opt(g1, 0, so("seq"), so("a sequence"));
      g1=argx_group_sequence(x);
        xs=argx_opt(g1, 0, so("int"), so("an int"));
           argx_type_int(xs, &seqi, 0);
        xs=argx_opt(g1, 0, so("string"), so("a string"));
           argx_type_so(xs, &seqs, 0);
        xs=argx_opt(g1, 0, so("option"), so("an option"));
           g2=argx_group_options(xs);
              xs=argx_opt(g2, 0, so("a"), so("a"));
              xs=argx_opt(g2, 0, so("b"), so("b"));
                 argx_type_so(xs, &seqs2, 0);
              xs=argx_opt(g2, 0, so("c"), so("c"));
        xs=argx_opt(g1, 0, so("flag"), so("a flag"));
           g2=argx_group_flags(xs);
              argx_flag(g2, &seqf0, &(bool){false}, so("x"), so("xxx"));
              argx_flag(g2, &seqf1, &(bool){false}, so("y"), so("yyy"));
              argx_flag(g2, &seqf2, &(bool){false}, so("z"), so("zzz"));
        xs=argx_opt(g1, 0, so("bool"), so("a bool"));
           argx_type_bool(xs, &seqb, 0);

    g1=argx_group(arg, so("other"));


#if 1
    Color col = COLOR_AQUA;


    x=argx_opt(g1, 0, so("color1"), so("color support"));
      argx_type_color(x, &col, 0);

    So_Fx fx = {0}, fx2 = {0};
    x=argx_opt(g1, 0, so("rice"), so("ricing support"));
      g2=argx_group_options(x);
        x=argx_opt(g2, 0, so("test"), so("test rice"));
          argx_builtin_opt_so_fx(x, &fx, 0);
        x=argx_opt(g2, 0, so("other"), so("other test rice"));
          argx_builtin_opt_so_fx(x, &fx2, 0);

    So subopt = SO, subopt2 = SO;
    x=argx_opt(g1, 0, so("subopt4"), so("some options"));
      g2=argx_group_options(x);
        x=argx_opt(g2, 0, so("subopt3"), so("second subopt"));
          g3=argx_group_options(x);
            x=argx_opt(g3, 0, so("subopt"), so("final subopt"));
              argx_type_so(x, &subopt, 0);
              //argx_builtin_switch(x, xs);
            x=argx_opt(g3, 0, so("subopt2"), so("final subopt2"));
              argx_type_so(x, &subopt2, 0);
        x=argx_opt(g2, 0, so("subopt2"), so(""));

    x=argx_opt(g1, '1', so("1111"), so("nothing"));
    x=argx_opt(g1, '2', so("somerandom"), so("nothing"));

    So key_v, key_r = so("invisible api key here");
    x=argx_env(arg, so("APIKEY"), so("test to hide api key visibility"));
      argx_type_so(x, &key_v, &key_r);
      argx_attr_hide(x, true);

#if 1
    bool posb = false;
    int pose;
    So soa = SO, sob = SO, soc = SO, sod = SO, soe = SO;
    x=argx_pos(arg, so("kind"), so("kind of thing to do"));
      argx_type_bool(x, &posb, 0);
    x=argx_pos(arg, so("number"), so("a number"));
      g2=argx_group_enum(x, &pose, 0);
        argx_enum_bind(g2, 0, so("zero"), SO);
        argx_enum_bind(g2, 1, so("one"), SO);
        argx_enum_bind(g2, 2, so("two"), SO);
    x=argx_pos(arg, so("alphabet"), so("id of thing"));
      g2=argx_group_options(x);
        x=argx_opt(g2, 0, so("a"), so("desc"));
          argx_type_so(x, &soa, 0);
        x=argx_opt(g2, 0, so("b"), so("desc"));
          argx_type_so(x, &sob, 0);
        x=argx_opt(g2, 0, so("c"), so("desc"));
          argx_type_so(x, &soc, 0);
        x=argx_opt(g2, 0, so("d"), so("desc"));
          argx_type_so(x, &sod, 0);
        x=argx_opt(g2, 0, so("e"), so("desc"));
          argx_type_so(x, &soe, 0);
#endif

    x=argx_opt(g1, 0, so("func"), so("function call"));
      argx_type_int(x, 0, 0);
      argx_callback(x, cbtest, 0, ARGX_PRIORITY_WHEN_ALL_VALID);

// => for flags:
//           argx_enum_all(g2, so("all"), so("enable all"));
//        x =argx_enum_unique(g2, so("default"), so("default option"));
//           argx_enum_bind(x, e1);
//           argx_enum_bind(x, e2);
#endif

    argx_builtin_rice(arg);

    So content = SO;
#if 0
    so_file_read(so("all.conf"), &content);
    arg_parse_config(arg, content, so("all.conf"));
#endif

    int status = 1;
    bool quit_early;
    if(arg_parse(arg, argc, argv, &quit_early)) goto clean;
    if(quit_early) goto clean;
    status = 0;

#if 0
    arg_help(arg);
    printff("::::::CONFIG:::::");
    arg_config(arg);
#endif

clean:
    so_free(&content);

    arg_free(&arg);
    vso_free(&vec_v);
    return status;
}

