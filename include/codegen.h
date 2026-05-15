#pragma once

#include <stddef.h>
#include <stdio.h>

#include "common.h"
#include "ast.h"

typedef struct
{
    char *name;
    char **instructions;
    size_t cap, len;
    ast_chunk_t *chunk;
} code_block_t;

typedef struct
{
    code_block_t *blocks;
    size_t cap, len;

    ast_chunk_t *chunk;
} codegen_t;

BF_API_HIDDEN void codegen_new(codegen_t *, ast_chunk_t *);
/// Use NULL to generate the root chunk.
BF_API_HIDDEN size_t codegen_generate(codegen_t *, ast_chunk_t *);
BF_API_HIDDEN void codegen_println(codegen_t *, size_t, char *);
BF_API_HIDDEN size_t codegen_allocate_section(codegen_t *, ast_chunk_t *);
BF_API_HIDDEN void codegen_process(codegen_t *, ast_node_t *, size_t);
BF_API_HIDDEN void codegen_dump(codegen_t);
BF_API_HIDDEN int codegen_flush(codegen_t, FILE *);
