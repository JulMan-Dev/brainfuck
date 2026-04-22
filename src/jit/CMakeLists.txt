if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(lib_sources
        codegen_aarch64.c)
endif()

set(lib_sources ${lib_sources} codegen_common.c)

target_sources(bf_int_static PRIVATE ${lib_sources})
target_sources(bf_int_shared PRIVATE ${lib_sources})
