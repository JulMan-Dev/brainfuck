#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "common.h"
#include "ast.h"

typedef struct
{
    const char *start;
    const char *ptr;

    size_t cap, size;
    ast_node_t *nodes;
} parser_t;

/// Creates a new parser. You must pass start pointer.
API_HIDDEN bool parser_new(parser_t *, const char *);

/// Consumes as much bytes as required to strip comment from ptr.
API_HIDDEN size_t parser_consume_comment(parser_t *);

/// Consumes as much bytes as required to generate a chunk.
API_HIDDEN size_t parser_consume_chunk(parser_t *, ast_chunk_t *);

/// Consumes as much bytes as required to generate a node.
API_HIDDEN size_t parser_consume_node(parser_t *, ast_node_t *);

/// Consumes as much bytes as required to generate a block.
API_HIDDEN size_t parser_consume_block(parser_t *, ast_chunk_t *);

/// Generates an error. Does not abort the program.
#define parser_error(parser, ...) do { \
    parser_error0(parser); \
    printf(__VA_ARGS__); \
    parser_error1(parser); \
} while (0)

API_HIDDEN void parser_error0(parser_t *), parser_error1(parser_t *);
