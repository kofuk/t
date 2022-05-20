project('fft', 'c', default_options : ['warning_level=3'])

compiler = meson.get_compiler('c')
math_dep = compiler.find_library('m')
fftw_dep = dependency('fftw3')
libadwaita_dep = dependency('libadwaita-1')

executable('fft', 'main.c', dependencies : [fftw_dep, math_dep, libadwaita_dep])
