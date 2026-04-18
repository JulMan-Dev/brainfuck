#include <stdio.h>

#include "ast.h"

API_HIDDEN void ast__inspect_node(ast_node_t const *node, size_t depth)
{
    for (size_t i = 0; i < depth; i++)
    {
        fprintf(stderr, "  ");
    }

    switch (node->kind)
    {
    case NODE_EOF:
        {
            fprintf(stderr, "<eof>\n");
        } break;

    case NODE_RIGHT:
    case NODE_LEFT:
    case NODE_PLUS:
    case NODE_MINUS:
    case NODE_OUTPUT:
    case NODE_INPUT:
        {
            fprintf(stderr, "%c (x%lu)\n", node->kind, node->operands);
        } break;

    case NODE_LOOP:
        {
            fprintf(stderr, "block [:\n");

            ast_chunk_t *chunk = node->chunk;
            for (size_t j = 0; j < chunk->nodes_count; j++)
            {
                ast__inspect_node(&chunk->nodes[j], depth + 1);
            }
        } break;
    }
}

void ast_inspect_chunk(ast_chunk_t const *chunk)
{
    for (size_t i = 0; i < chunk->nodes_count; i++)
    {
        ast__inspect_node(&chunk->nodes[i], 0);
    }
}
