# rlarg

Argument parser for C.

## Features

**Core**

- directly assign parsed values to your variables
- positional values, optional values and environmental values
- supports `bool`, `int`, `ssize_t`, `So` (string), `Color`, `enum`, `flags` (toggle), `group` (sub-options)
- disables all colors when piping (`isatty() == 0`)

**Builtin**

- `arg_builtin_opt_help`: expansive help
- `arg_builtin_env_compgen`: auto completion support (see also [`bash/rlc`](bash/rlc))
- `arg_builtin_env_nocolor`: explicit `NOCOLOR` support 
- `arg_builtin_opt_source`: config loading support 
- `arg_enable_config_print`: config generation support
- `argx_builtin_rice`: enable colors

**Miscellaneous**

- `argx_callback`: provides support for custom callback (e.g. custom parser)
- `argx_attr_hide`: hides a value in help listing (e.g. API key)
- `argx_attr_configurable`: control if value can be configured via config file

## Adding an internal type

1. [`argx.h`](rlarg/argx.h) extend enum and union:
    - `Argx_Type_List`
    - `Argx_Value_Union`
2. [`argx.c`](rlarg/argx.c) extend core: (expand switches)
    - `argx_free`
    - `argx_so`
3. [`arg-parse.c`](rlarg/arg-parse.c) specify conversion functions: (expand array of callbacks)
    - `static_parse_argx_single_cbs`
    - `static_parse_argx_vector_vals_cbs`
    - `static_parse_argx_vector_cbs`
4. [`arg-parse.c`](rlarg/arg-parse.c) fix setref support: (expand switches)
    - `arg_parse_setref_argx`
5. [`arg-compgen.c`](rlarg/arg-compgen.c) add compgen support: (expand switch)
    - `static_arg_compgen_argx`
6. [`argx-type.c`](rlarg/argx-type.c) add the actual code to specify your type: (create your function)
    - `argx_type_???`

You can alternatively view places you need to patch via:

```sh
$ grep ARGX_TYPE_STRING * -rn
```

