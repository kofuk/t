project('gobj', 'c', default_options : ['warning_level=3'])

glib_deps = [dependency('glib-2.0'), dependency('gobject-2.0')]

executable(
  meson.project_name(),
  'main.c', 'hoge.c',
  dependencies : glib_deps)
