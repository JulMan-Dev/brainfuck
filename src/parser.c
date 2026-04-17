#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

bool parser_new(parser_t *out, const char *start)
{
    *out = (parser_t) {
        .start = start,
        .ptr = start,
    };
    return true;
}

size_t parser_consume_chunk(parser_t *parser, ast_chunk_t *out)
{
    // allocating by blocks of 16
    size_t cap, count = 0;
    ast_node_t *list = calloc(cap = 16, sizeof(ast_node_t));
    assert(list);
    ast_node_t last;
    size_t bytes = 0;

    do
    {
        bytes += parser_consume_node(parser, &last);

        if (cap == count)
        {
            cap += 16;
            list = realloc(list, cap * sizeof(ast_node_t));
            assert(list);
        }

        list[count++] = last;
    }
    while (last.kind != NODE_EOF);

    *out = (ast_chunk_t) {
        .nodes_count = count,
        .nodes = list,
    };
    return bytes;
}

API_HIDDEN bool parser_is_bf(char code)
{
    return code == '<' || code == '>' || code == '+' || code == '-' || code == '[' || code == ']';
}

size_t parser_consume_comment(parser_t *parser)
{
    const char *start = parser->ptr;

    while (*parser->ptr != 0 && !parser_is_bf(*parser->ptr))
    {
        parser->ptr++;
    }

    return parser->ptr - start;
}

size_t parser_consume_node(parser_t *parser, ast_node_t *out)
{
    const char *real_start = parser->ptr;

    char current = 0;
    const char *start;
    size_t operands = 0;

    while (*parser->ptr != 0)
    {
        parser_consume_comment(parser);

        if (*parser->ptr == 0)
        {
            break;
        }

        if (current == 0)
        {
            if (*parser->ptr == '[')
            {
                start = parser->ptr++;

                ast_chunk_t *chunk = malloc(sizeof(ast_chunk_t));
                size_t bytes = parser_consume_block(parser, chunk);

                if (chunk->nodes[chunk->nodes_count - 1].kind == NODE_EOF)
                {
                    parser->ptr = start; // seeking to start
                    parser_error(parser, "unterminated block");
                    abort();
                }

                // we are sure the last is ']'

                *out = (ast_node_t) {
                    .kind = NODE_LOOP,
                    .source = start,
                    .block = {
                        .chunk = chunk,
                        .end_binding = 1,
                    },
                };
                return real_start - start + bytes;
            }

            if (*parser->ptr == ']')
            {
                parser_error(parser, "found ]");
                abort();
            }

            current = *parser->ptr;
            start = parser->ptr;
            operands++;
            parser->ptr++;
        }
        else if (current == *parser->ptr)
        {
            operands++;
            parser->ptr++;
        }
        else
        {
            break;
        }
    }

    if (!current)
    {
        *out = (ast_node_t) {
            .kind = NODE_EOF,
            .source = parser->ptr,
        };
        return parser->ptr - real_start;
    }

    // end of the node
    *out = (ast_node_t) {
        .kind = current,
        .source = start,
        .operands = operands,
    };
    return parser->ptr - real_start;
}

size_t parser_consume_block(parser_t *parser, ast_chunk_t *out)
{
    size_t cap, count = 0;
    ast_node_t *list = calloc(cap = 16, sizeof(ast_node_t));
    assert(list);
    ast_node_t last;
    size_t bytes = 0;

    while (*parser->ptr != 0)
    {
        bytes += parser_consume_comment(parser);

        if (*parser->ptr == 0)
        {
            break;
        }

        if (*parser->ptr == ']')
        {
            bytes += 1;
            parser->ptr++;
            break;
        }

        bytes += parser_consume_node(parser, &last);

        if (cap == count)
        {
            cap += 16;
            list = realloc(list, cap * sizeof(ast_node_t));
            assert(list);
        }

        list[count++] = last;
    }

    *out = (ast_chunk_t) {
        .nodes_count = count,
        .nodes = list,
    };
    return bytes;
}

void parser_error0(parser_t *parser)
{
    printf("Error on :%lu: ", parser->ptr - parser->start);
}

void parser_error1(parser_t *parser)
{
    printf("\n");
}
