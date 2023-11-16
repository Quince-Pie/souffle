project('souffle', 'c',
     default_options : ['warning_level=3', 'c_std=c2x', 'optimization=3'],
    version: '1.3.0')


if get_option('no_color')
  add_project_arguments('-DSOUFFLE_NOCOLOR', language: 'c')
endif

souffle_srcs = files(['src/souffle.c', 'src/hashy.c'])
souffle_inc = include_directories('src')

souffle_lib = library('souffle', souffle_srcs, include_directories : [souffle_inc])

link_args = []
if host_machine.system() == 'darwin'
  link_args += ['-Wl,-undefined,dynamic_lookup']
endif


souffle_dep = declare_dependency(
    link_args: link_args,
    include_directories : [souffle_inc],
    link_with : [souffle_lib],
)
