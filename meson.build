project('gcmp', 'c', version: '21.01', default_options: [ 'warning_level=2', 'c_std=gnu11'])

bindir = join_paths(get_option('prefix'), get_option('bindir'))

cc = meson.get_compiler('c')

c_args = ['-DVERSION="' + meson.project_version() + '"']

mpfr_dep = cc.find_library('mpfr', required: true)

res = []

subdir('data')

c = run_command('sh', '-c', 'for file in src/*.h src/*.c; do echo $file; done')
gcm_src = c.stdout().strip().split('\n')

gcm_src += res

gcm_deps = [
  mpfr_dep,
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22')
]

executable(meson.project_name(), gcm_src, dependencies: gcm_deps, c_args: c_args, install: true)

