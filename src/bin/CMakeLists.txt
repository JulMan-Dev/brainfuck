target_sources(bf PRIVATE
    brainfuck.c)
target_link_libraries(bf bf_int_static)

target_sources(bfc PRIVATE
    brainfuckc.c)
