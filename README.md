# rlarg

Argument parser for C.

## Adding an internal type

1. [`argx.h`](rlarg/argx.h) extend enum:
    - `Argx_Type_List`
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
6. [`arg-type.c`](rlarg/arg-type.c) add the actual code to specify your type: (create your function)
    - `argx_type_???`

You can alternatively view places you need to patch via:

```sh
$ grep ARGX_TYPE_STRING * -rn
```

