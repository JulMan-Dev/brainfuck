#pragma once

#include <stddef.h>

#include "common.h"

typedef struct ast_node_t ast_node_t;

typedef struct
{
    size_t nodes_count;
    ast_node_t *nodes;
} ast_chunk_t;

typedef enum
{
    NODE_EOF = 0,
    NODE_RIGHT = '>',
    NODE_LEFT = '<',
    NODE_PLUS = '+',
    NODE_MINUS = '-',
    NODE_OUTPUT = '.',
    NODE_INPUT = ',',
    NODE_LOOP = '[',
} ast_kind_t;

struct ast_node_t
{
    ast_kind_t kind;
    const char *source;

    union {
        // RIGHT, LEFT, PLUS, MINUS, OUTPUT, INPUT
        size_t operands;

        // LOOP
        ast_chunk_t *chunk;
    };
};

/// Prints out the AST chunk.
API_HIDDEN void ast_inspect_chunk(ast_chunk_t const *);
