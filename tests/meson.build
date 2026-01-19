should_pass = [
  'all.c',
  ]
should_fail = [
  'fail-duplicate.c',
  'fail-duplicate2.c',
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

endif

