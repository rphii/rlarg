project('rlarg', 'c',
  meson_version: '>=1.1')

subdir('src')
subdir('bash')

pkg = import('pkgconfig')
pkg.generate(
  libraries: rlarg,
  version: '9999',
  name: 'rlarg',
  description: 'Argument parser for C.',
  )

dep_rlarg = declare_dependency(link_with: rlarg)

