#include "arg-parse-config.h"
#include "arg-parse.h"

#define TODO_WARN  \
    printff(F("TODO WARN", FG_YL BOLD))

#define TODO_ERROR  \
    printff(F("TODO ERROR", FG_RD BOLD))

Argx *arg_parse_config_get_hierarchy(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    So *tmp = &p->tmp_full_hierarchy;
    so_clear(tmp);
    if(so_len(p->section)) {
        so_extend(tmp, p->section);
        so_push(tmp, '.');
    }
    so_extend(tmp, p->hierarchy);
    arg_parse_error_allow_more(&p->stream);
    p->stream.source.number = head->line_number;
    Argx *argx = arg_parse_hierarchy(p->arg, &p->stream, *tmp, 0);
    if(!argx) {
        p->status |= ARG_PARSE_CONFIG_ERR_HIERAR;
    }
    //printff("FULL HIER: %.*s -> %p", SO_F(p->tmp_full_hierarchy), argx);
    return argx;
}

int arg_parse_config_assign_file_named(Arg_Parse_Config *p, So path, bool in_array) {
    if(!p->argx) {
        //TODO_WARN;
        return -1;
    }
    /* construct path */
    So content = SO;
    /* TODO: the tmp_file_path is lost to the operator of the arg parser (it is technically a source..) */
    so_clear(&p->tmp_file_path_wordexp);
    so_clear(&p->tmp_file_path);
    so_extend_wordexp(&p->tmp_file_path_wordexp, path, false);
    if(so_at0(p->tmp_file_path_wordexp) == PLATFORM_CH_SUBDIR) {
        so_extend(&p->tmp_file_path, p->tmp_file_path_wordexp);
    } else {
        so_path_join(&p->tmp_file_path, so_get_dir(p->stream.source.path), path);
    }
    /* read file */
    //printff("FILE NAMED %.*s", SO_F(p->tmp_file_path));
    if(so_file_read(p->tmp_file_path, &content)) {
        Argx pseudo = { .opt = p->tmp_file_path };
        arg_parse_error(p->arg, &p->stream, ARG_PARSE_ERROR_INVALID_FILE, &pseudo);
        p->status |= ARG_PARSE_CONFIG_ERR_FILE;
        return -1;
    }
    /* now parse */
    vso_push(&p->arg->builtin.sources_content, content);
    if(in_array) {
        p->stream.carg = content;
        if(arg_parse_argx(p->arg, &p->stream, p->argx, content)) {
            p->status |= ARG_PARSE_CONFIG_ERR_ASSIGN;
        }
    } else {
        for(So line = SO; so_splice(content, &line, '\n'); ) {
            if(so_is_zero(line)) continue;
            line = so_trim(line);
            if(!so_len(line)) continue;
            p->stream.carg = line;
            if(arg_parse_argx(p->arg, &p->stream, p->argx, line)) {
                p->status |= ARG_PARSE_CONFIG_ERR_ASSIGN;
            }
        }
    }
    return 0;
}

int arg_parse_config_assign_file_auto(Arg_Parse_Config *p, bool in_array) {
    arg_parse_config_assign_file_named(p, p->tmp_full_hierarchy, in_array);
    return 0;
}

int arg_parse_config_assign_string(Arg_Parse_Config *p, bool in_array) {
    if(!p->argx) {
        //TODO_WARN;
        return -1;
    }
    //printff("STRING %.*s", SO_F(val));
    p->stream.carg = p->tmp_string;
    if(arg_parse_argx(p->arg, &p->stream, p->argx, p->tmp_string)) {
        p->status |= ARG_PARSE_CONFIG_ERR_ASSIGN;
    }
    vso_push(&p->arg->builtin.sources_content, p->tmp_string);
    p->tmp_string = SO;
    return 0;
}

int arg_parse_config_assign_other(Arg_Parse_Config *p, So val, bool in_array) {
    if(!p->argx) {
        //TODO_WARN;
        return -1;
    }
    //printff("OTHER %.*s", SO_F(val));
    p->stream.carg = val;
    if(arg_parse_argx(p->arg, &p->stream, p->argx, val)) {
        p->status |= ARG_PARSE_CONFIG_ERR_ASSIGN;
    }
    return 0;
}

void arg_parse_config_shift(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, size_t n) {
    size_t len = so_len(head->so);
    if(len < n) {
        ABORT(ERR_UNREACHABLE("can't shift this much: %zu/%zu"), n, len);
    }
    size_t n_newline = so_find_ch(so_iE(head->so, n), '\n');
    if(n_newline < n) ++head->line_number;
    so_shift(&head->so, n);
}

/* return true on match */
bool arg_parse_config_ch(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, char c) {
    ASSERT_ARG(p);
    if(!head->so.len) return false;
    bool result = (bool)(*head->so.str == c);
    if(result) {
        arg_parse_config_shift(p, head, 1);
    }
    return result;
}

/* return true on match */
bool arg_parse_config_any(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, char *s) {
    ASSERT_ARG(p);
    if(!head->so.len) return false;
    char *result = strchr(s, *head->so.str);
    if(result && *result) {
        arg_parse_config_shift(p, head, 1);
        return true;
    }
    return false;
}

void arg_parse_config_ws(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    while(arg_parse_config_any(p, head, " \t\v\n\r")) {}
}

void arg_parse_config_ws_no_newline(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    while(arg_parse_config_any(p, head, " \t\v\r")) {}
}

void arg_parse_config_other(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, So *val, bool in_array) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    ASSERT_ARG(val);
    size_t len = so_find_ch(head->so, '\n');
    size_t len_to_comment = SIZE_MAX;
    size_t len_to_arr_delim = SIZE_MAX;
    if(in_array && len) {
        So check_for_comma = so_iE(head->so, len);
        size_t len_to_comma = so_find_ch(check_for_comma, ',');
        size_t len_to_brack = so_find_ch(check_for_comma, ']');
        len_to_arr_delim = len_to_comma < len_to_brack ? len_to_comma : len_to_brack;
    }
    if(len < so_len(head->so)) {
        len_to_comment = so_find_ch(head->so, '#');
        size_t less = len_to_comment < len_to_arr_delim ? len_to_comment : len_to_arr_delim;
        if(len < less) less = len;
        *val = so_trim(so_iE(head->so, less));
    }
    if(len_to_comment != SIZE_MAX && len_to_arr_delim != SIZE_MAX && len_to_arr_delim < len_to_comment) {
        arg_parse_config_shift(p, head, len_to_arr_delim);
    } else {
        arg_parse_config_shift(p, head, len);
    }
}

bool arg_parse_config_hierarchy(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = false;
    arg_parse_config_ws(p, head);
    so_clear(&p->hierarchy);
    Arg_Parse_Config_Head r = *head;
    while(head->so.len) {
        arg_parse_config_ws_no_newline(p, head);
        if(arg_parse_config_ch(p, head, '\n')) break;
        if(arg_parse_config_ch(p, head, '=')) { ok = true; break; }
        so_push(&p->hierarchy, so_at0(head->so));
        arg_parse_config_shift(p, head, 1);
    }
    if(ok) {
        if(so_len(p->hierarchy)) {
            //printff("HIERARCHY %.*s", SO_F(p->hierarchy));
        }
    } else {
        Argx pseudo = { .opt = r.so };
        p->stream.source.number = r.line_number;
        arg_parse_error(p->arg, &p->stream, ARG_PARSE_ERROR_MISSING_HIERARCHY_DELIM, &pseudo);
        p->status |= ARG_PARSE_CONFIG_ERR_SYNTAX;
    }
    return ok;
}

bool arg_parse_config_string(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, bool in_array) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = false;
    Arg_Parse_Config_Head q = *head;
    So origin = q.so;
    if(!arg_parse_config_ch(p, &q, '"')) return false;
    size_t n_unescaped = 0;
    so_clear(&p->tmp_string);
    if(!so_fmt_unescape(&p->tmp_string, q.so, so("\""), so("\n"), &n_unescaped)) {
        arg_parse_config_shift(p, &q, n_unescaped);
        if(arg_parse_config_ch(p, &q, '"')) ok = true;
    }
    if(!ok) {
        Argx pseudo = { .opt = so_split_ch(origin, '\n', 0) };
        arg_parse_error(p->arg, &p->stream, ARG_PARSE_ERROR_MISSING_STRING_DELIM, &pseudo);
        p->status |= ARG_PARSE_CONFIG_ERR_SYNTAX;
        return false;
    }
    if(ok) {
        *head = q;
    }
    return ok;
}

bool arg_parse_config_file(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, bool in_array) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    Arg_Parse_Config_Head q = *head;
    if(!arg_parse_config_ch(p, &q, 'f')) return false;
    if(!arg_parse_config_ch(p, &q, 'i')) return false;
    if(!arg_parse_config_ch(p, &q, 'l')) return false;
    if(!arg_parse_config_ch(p, &q, 'e')) return false;
    arg_parse_config_ws_no_newline(p, &q);
    so_clear(&p->file);
    bool used_path = false;
    if(arg_parse_config_ch(p, &q, '(')) {
        ok = false;
        arg_parse_config_ws_no_newline(p, &q);
        /* get string to path */
        ok = true;
        if(so_len(q.so) && so_at0(q.so) == '"') {
            if(!arg_parse_config_string(p, &q, in_array)) ok = false;
        }
        if(ok) {
            used_path = true;
            /* got it, find ')' */
            arg_parse_config_ws_no_newline(p, &q);
            if(!arg_parse_config_ch(p, &q, ')')) ok = false;
        }
    }
    if(!ok) {
        Argx pseudo = { .opt = (p->argx ? p->argx->opt : so("???")), .desc = so_split_ch(head->so, '\n', 0) };
        p->stream.source.number = q.line_number - 1; /* we already skipped to the next line, do: - 1 */
        arg_parse_error(p->arg, &p->stream, ARG_PARSE_ERROR_MISSING_FILE_DELIM, &pseudo);
        p->status |= ARG_PARSE_CONFIG_ERR_SYNTAX;
        /* shift until next line */
        arg_parse_config_shift(p, &q, so_find_ch(q.so, '\n'));
        *head = q;
    } else {
        *head = q;
        if(used_path) arg_parse_config_assign_file_named(p, p->tmp_string, in_array);
        else arg_parse_config_assign_file_auto(p, in_array);
    }
    return ok;
}

bool arg_parse_config_value(Arg_Parse_Config *p, Arg_Parse_Config_Head *head, bool in_array) {
    bool ok = false;
    bool shift = false;
    So other = SO;
    Arg_Parse_Config_Head q = *head;
    arg_parse_config_ws(p, &q);
    if(arg_parse_config_file(p, &q, in_array)) {
        ok = true;
    } else if(arg_parse_config_string(p, &q, in_array)) {
        arg_parse_config_assign_string(p, in_array);
        ok = true;
    } else {
        arg_parse_config_other(p, &q, &other, in_array);
        if(so_len(other)) {
            /* ONLY assign this, if no error reported! */
            if(!p->stream.error_id) {
                arg_parse_config_assign_other(p, other, in_array);
            }
            ok = true;
        }
        shift = true;
    }
    arg_parse_config_ws(p, &q);
    if(ok || shift) {
        *head = q;
    }
    return ok;
}

bool arg_parse_config_section(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = false;
    arg_parse_config_ws(p, head);
    Arg_Parse_Config_Head r = *head;
    if(!arg_parse_config_ch(p, head, '[')) return false;
    so_clear(&p->section);
    arg_parse_config_ws_no_newline(p, head);
    while(head->so.len) {
        arg_parse_config_ws_no_newline(p, head);
        if(arg_parse_config_ch(p, head, '\n')) break;
        if(arg_parse_config_ch(p, head, ']')) { ok = true; break; }
        so_push(&p->section, so_at0(head->so));
        arg_parse_config_shift(p, head, 1);
    }
    arg_parse_config_ws_no_newline(p, head);
    So rest = SO;
    arg_parse_config_other(p, head, &rest, false); /* this is guaranteed to go to the next line */
    if(so_len(rest)) ok = false;
    /* TODO if one section is wrong, skip all of it under that ?? until next section ? */
    if(ok) {
        //printff("SECTION %.*s", SO_F(p->section));
    } else {
        so_clear(&p->section);
        Argx pseudo = { .opt = so_split_ch(r.so, '\n', 0) };
        p->stream.source.number = r.line_number;
        arg_parse_error(p->arg, &p->stream, ARG_PARSE_ERROR_INVALID_SECTION, &pseudo);
        p->status |= ARG_PARSE_CONFIG_ERR_SYNTAX;
    } 
    return ok;
}

bool arg_parse_config_array(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = false;
    Arg_Parse_Config_Head q = *head;
    if(!arg_parse_config_ch(p, &q, '[')) return false;
    //printff("PARSE ARRAY vvvvvvvv");
    arg_parse_config_ws(p, &q);
    size_t values_parsed_old = 0;
    size_t values_parsed_now = 0;
    while(q.so.len) {
        arg_parse_config_ws(p, &q);
        //printff("ARRAY PASS %zu/%zu: %.*s",values_parsed_now,values_parsed_old,SO_F(so_split_ch(q.so,'\n',0)));
        /* can we expect an end ']' or comma ',' ? */
        if(values_parsed_now > values_parsed_old) {
            /* .. comments .. */
            if(arg_parse_config_ch(p, &q, '#')) {
                /* skip whole line */
                arg_parse_config_shift(p, &q, so_find_ch(q.so, '\n'));
            } else if(!arg_parse_config_ch(p, &q, ',')) {
                if(arg_parse_config_ch(p, &q, ']')) { ok = true; break; }
                else { break; }
            } else {
                arg_parse_error_allow_more(&p->stream);
                values_parsed_old = values_parsed_now;
            }
        } else {
            if(arg_parse_config_ch(p, &q, ']')) { ok = true; break; }
            if(arg_parse_config_ch(p, &q, ',')) {
                Argx pseudo = { .opt = so_split_ch(head->so, '\n', 0) };
                p->stream.source.number = q.line_number;
                arg_parse_error(p->arg, &p->stream, ARG_PARSE_ERROR_MISSING_ARRAY_VALUE, &pseudo);
                p->status |= ARG_PARSE_CONFIG_ERR_SYNTAX;
                break;
            }
            /* try parse a value */
            values_parsed_old = values_parsed_now;
            //printff("1EAD:%.*s",SO_F(q.so));
            if(arg_parse_config_value(p, &q, true)) {
                ++values_parsed_now;
            }
            //printff("HEAD:%.*s",SO_F(q.so));
        }
    }
    if(!ok) {
        Argx pseudo = { .opt = p->argx ? p->argx->opt : so("???"), .desc = so_split_ch(head->so, '\n', 0) };
        arg_parse_error(p->arg, &p->stream, ARG_PARSE_ERROR_MISSING_ARRAY_DELIM, &pseudo);
        p->status |= ARG_PARSE_CONFIG_ERR_SYNTAX;
        /* shift until next line */
        arg_parse_config_shift(p, &q, so_find_ch(q.so, '\n'));
        *head = q;
        ok = false;
    } else {
        *head = q;
    }
    //printff("PARSE ARRAY ^^^^^^^^ --- ok? %u", ok);
    //printff("REST:[%.*s]",SO_F(*head));
    return ok;
}

bool arg_parse_config_config(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    Arg_Parse_Config_Head q = *head;
    arg_parse_config_ws(p, &q);
    //printff("PARSE CONFIG");
    if(arg_parse_config_array(p, &q)) {
    } else if(arg_parse_config_value(p, &q, false)) {
    } else {
        arg_parse_error(p->arg, &p->stream, ARG_PARSE_ERROR_CONFIG, p->argx);
        p->status |= ARG_PARSE_CONFIG_ERR_CONFIG;
        ok = false;
    }
    arg_parse_config_ws(p, &q);
    if(ok) {
        *head = q;
    }
    return ok;
}

bool arg_parse_config_settings(Arg_Parse_Config *p, Arg_Parse_Config_Head *head) {
    ASSERT_ARG(p);
    ASSERT_ARG(head);
    bool ok = true;
    So inspect = SO;
    Arg_Parse_Config_Head q = *head;
    arg_parse_config_other(p, &q, &inspect, false);
    if(!so_len(inspect)) return false;
    Arg_Parse_Config_Head inspected = { .so = inspect, .line_number = head->line_number };
    if(!arg_parse_config_hierarchy(p, &inspected)) return false;

    /* now we have the full hierarchy, assign argx */
    //printff(">>>UPDATE HIERARCHY");
    p->argx = arg_parse_config_get_hierarchy(p, head);

    /* start of inspected now points to where we want to continue! */
    arg_parse_config_shift(p, head, inspected.so.str - head->so.str);

    arg_parse_config_config(p, head);

    /* if an error occured, skip this line */
    if(p->stream.error_id) {
        arg_parse_config_shift(p, head, so_find_ch(head->so, '\n'));
    }

    return ok;
}

int arg_parse_config(struct Arg *arg, So config, So path) {
    arg->help.error = 0;
    arg->help.last = 0;

    So comment = SO;

    Arg_Parse_Config_Head head = {
        .so = config,
        .line_number = 1,
    };

    Arg_Parse_Config p = {
        .arg = arg,
        .stream.is_config = true,
        .stream.source.id = ARG_STREAM_SOURCE_CONFIG,
        .stream.source.path = path,
        .stream.source.number = 1,
    };

    while(head.so.len) {
        if(!arg_parse_config_section(&p, &head)) {
            if(!arg_parse_config_settings(&p, &head)) {
                arg_parse_config_other(&p, &head, &comment, false);
            }
        }
    }

    arg_stream_free(&p.stream);

    so_free(&p.tmp_file_path);
    so_free(&p.tmp_file_path_wordexp);
    so_free(&p.tmp_full_hierarchy);
    so_free(&p.tmp_string);
    so_free(&p.file);
    so_free(&p.hierarchy);
    so_free(&p.section);

    if(p.status) p.status |= ARG_PARSE_CONFIG_ERR_CONFIG;

    return p.status;
}


