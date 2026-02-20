should_pass = [
  'all.c',
  'readme.c',
  ]
should_fail = [
  'fail-duplicate.c',
  'fail-duplicate2.c',
  ]

configs = [
    [true,  'empty.conf'],
    [true,  'scope1.conf'],
    [false, 'scope2.conf'],
    [true,  'empty-string.conf'],
    [true,  'empty-array.conf'],
    [true,  'file1.conf'],
    [true,  'file2.conf'],
    [false, 'array-whitespace.conf'],
    [true,  'array-whitespace2.conf'],
  ]

dep_rlc = dependency('rlc', required: true)
dep_rlso = dependency('rlso', required: true)

if get_option('tests').enabled()

  tests = []
  foreach ex : should_pass
    tests += executable(ex, ex, link_with: librlarg, dependencies: [dep_rlc, dep_rlso])
  endforeach

  foreach ex : tests
    test(ex.name(), ex)
  endforeach

  tests = []
  foreach ex : should_fail
    tests += executable(ex, ex, link_with: librlarg, dependencies: [dep_rlc, dep_rlso])
  endforeach

  foreach ex : tests
    test(ex.name(), ex, should_fail: true)
  endforeach

  config_checker = executable('parse-config', 'parse-config.c', link_with: librlarg, dependencies: [dep_rlc, dep_rlso])
  foreach cfg : configs
    test('config-checker // ' + cfg[1], config_checker, args: [ meson.current_source_dir() / 'parse-config' / cfg[1] ], should_fail: not cfg[0])
  endforeach


endif

