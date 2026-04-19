set(lib_sources
    runtime.c)

target_sources(bf_rt_static PRIVATE ${lib_sources})
target_sources(bf_rt_shared PRIVATE ${lib_sources})
