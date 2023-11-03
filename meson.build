project('souffle', 'c',
     default_options : ['warning_level=3', 'c_std=c2x', 'optimization=3'],
    version: '0.100.0')


souffle_srcs = files('src/souffle.c')
souffle_inc = include_directories('src')

khash_inc = include_directories('src/klib')

souffle_lib = library('souffle', souffle_srcs, include_directories : [souffle_inc, khash_inc])

souffle_dep = declare_dependency(
    include_directories : [souffle_inc],
    link_with : [souffle_lib],
)
