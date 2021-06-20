project(
  'gres', 'c',
  default_options : ['warning_level=3'])

gtk_dep = dependency('gtk4')

gnome = import('gnome')

resources = gnome.compile_resources(
  'resources', 'gres.gresource.xml')
executable(
  'gres', 'main.c', resources,
  dependencies : gtk_dep)
