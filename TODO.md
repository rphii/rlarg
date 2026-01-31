
- coloring / ricing + do not color if redirect?
- parse files and add sources
- add ability to hide groups in help?
- codify usage examples
- better compgen for flags, e.g: +flag,-flag2 and show combinations..
- formatting of arrays to multiple lines
- config things:
    - do not output # flag if it takes no values
    - add option to disable config generation entirely (e.g. --help should not go into config)
    - add option to disable/enable function execution during compgen
    - do not allow config to have --option = <no-value> with a flag without a value? (what if it has cb?)
    - proper parsing of strigs in configs
    - proper parsing of multiline arrays in configs
    - allow parsing of +value or -value for arrays...?
    - allow something like this: CONFIG_PRINT=hierarchy ./program -> only prints the config for that hierarchy/group!

