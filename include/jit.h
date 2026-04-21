#pragma once

#include <stddef.h>

#include "libbf/rt.h"
#include "__int_state.h"
#include "ast.h"

typedef struct
{
    size_t rel;
    void *abs;
    uint64_t data;
} patch_t;

typedef struct
{
    ast_chunk_t *chunk;
    unsigned char *generated;
    size_t cap, len;
    patch_t *patches;
    size_t pcap, plen;
    lib_symbols_t *symbols;
} jit_codegen_t;

API_HIDDEN int jit_generator_new(jit_codegen_t *, lib_symbols_t *, ast_chunk_t *);
API_HIDDEN int jit_generator_prepare(jit_codegen_t *);
API_HIDDEN int jit_generator_patch(jit_codegen_t *, unsigned char *);
API_HIDDEN size_t jit_generator_code_len(jit_codegen_t const *);
API_HIDDEN int jit_generator_flush(jit_codegen_t *, unsigned char *, size_t);
API_HIDDEN int jit_generator_free(jit_codegen_t const *);

// Not intended to be called directly in C. Written in Assembly for JIT-generated code.
API_HIDDEN int __jit_builtin_error(void);
