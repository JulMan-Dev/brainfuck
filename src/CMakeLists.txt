add_subdirectory(bin)
add_subdirectory(librt)

target_sources(bf PRIVATE
    parser.c
    ast.c
    runtime.c)

target_sources(bfc PRIVATE
    parser.c
    ast.c
    codegen.c)
