set(lib_sources
        bf.c)

target_sources(bf_int_shared PRIVATE ${lib_sources})
target_sources(bf_int_static PRIVATE ${lib_sources})
