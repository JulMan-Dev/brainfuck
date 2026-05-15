set(lib_sources
    state.c
    eval.c
    strip.c
    code.c)

target_sources(bf_int_static PRIVATE ${lib_sources})
target_sources(bf_int_shared PRIVATE ${lib_sources})
