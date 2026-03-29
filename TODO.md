# priority

- most recent source is wrong ( -> sort by nb_source )
- printing colors... remove the color.. in CONFIG_PRINT..
- organize so_fmt_fx better..... (they all over the place)
- better compgen for flags, e.g: +flag,-flag2 and show combinations..

- more tests, e.g.
    - CONFIG_PRINT=section -> input to same thing -> expect same CONFIG_PRINT=section output
    - parse basic types

# later

- min/max for numbers and...
- ..float/double
- ..time

- hot/reload
- note that if URI wasn't found? optional enable/disable?
- don't output 'not set anywhere' .. if a group? what if callback? or just list sources of all sub-items?

# later later

*what should some these even be ??*

- IMMEDIATELY_PRE / POST
- compgen for, e.g. `prog --help<tab>` does not finish the word :(
- parse arbitrary config hierarchy -> `arg-parse-config.c:30`
- add option to set parser error... in arg_runtime?
- roff (man page) generation?

