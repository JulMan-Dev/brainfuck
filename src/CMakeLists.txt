add_subdirectory(bin)

target_sources(bf PRIVATE
    parser.c
    ast.c
    runtime.c)
