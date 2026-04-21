add_subdirectory(bin)
add_subdirectory(librt)
add_subdirectory(libint)
add_subdirectory(jit)

target_sources(bfc PRIVATE
    parser.c
    ast.c
    codegen.c)

set(lib_sources
    parser.c
    ast.c)

target_sources(bf_int_static PRIVATE ${lib_sources})
target_sources(bf_int_shared PRIVATE ${lib_sources})
