#include "arg.h"

void arg_after_fmt_config_available(So *out, struct Arg *arg) {

    Argx_Group **itE = array_itE(arg->opts);
    for(Argx_Group **it = arg->opts; it < itE; ++it) {
        so_extend(out, (*it)->name);
        if(it + 1 < itE) so_push(out, ',');
    }

}

int arg_after_fmt_config(So *out, struct Arg *arg, So configs, Arg_Builtin_Color_List color) {
    int err = 0;

    Arg_Builtin_Color_List color_prev = arg->builtin.color;
    arg->builtin.color = color;
    arg_update_color_off(arg);


    So config = SO;
    while(so_splice(configs, &config, ',')) {
        Argx_Group *group = argx_group_get_opt(arg, config);
        if(group) {
            argx_group_fmt_config(out, group);
        } else {
            err += 1;
        }
    }

    arg->builtin.color = color_prev;
    return err;
}

