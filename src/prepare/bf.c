#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "ast.h"
#include "libbf/int.h"
#include "__int_state.h"
#include "__int_code.h"

struct __bf_parser
{
    const char *start, *ptr;
    size_t len;

    // error
    size_t error_offset;
    const char *error_message;
};

static int parser_new(struct __bf_parser *parser, const char *start, size_t len)
{
    *parser = (struct __bf_parser) {
        .start = start,
        .ptr = start,
        .len = len,
        .error_offset = 0,
        .error_message = NULL,
    };
    return 0;
}

static int parser_is_bf(char code)
{
    return code == '<' || code == '>' || code == '+' || code == '-' || code == '.' || code == ',' ||
        code == '[' || code == ']';
}

static int parser_consume_comment(struct __bf_parser *, size_t *);
static int parser_consume_node(struct __bf_parser *, ast_node_t *, size_t *);
static int parser_consume_chunk(struct __bf_parser *, ast_chunk_t *, size_t *);
static int parser_consume_block(struct __bf_parser *, ast_chunk_t *, size_t *);

int parser_consume_comment(struct __bf_parser *parser, size_t *out)
{
    const char *start = parser->ptr;

    while (parser->ptr - parser->start <= parser->len && !parser_is_bf(*parser->ptr))
    {
        parser->ptr++;
    }

    if (out)
    {
        *out = parser->ptr - start;
    }
    return 0;
}

int parser_consume_node(struct __bf_parser *parser, ast_node_t *out, size_t *out_size)
{
    const char *real_start = parser->ptr;
    int error;

    char current = 0;
    const char *start;
    size_t operands = 0;

    while (parser->ptr - parser->start <= parser->len)
    {
        parser_consume_comment(parser, NULL);

        if (parser->ptr - parser->start > parser->len)
        {
            break;
        }

        if (current == 0)
        {
            if (*parser->ptr == '[')
            {
                start = parser->ptr++;

                ast_chunk_t *chunk = malloc(sizeof(ast_chunk_t));

                if (!chunk)
                {
                    return ENOMEM;
                }

                size_t bytes;
                if ((error = parser_consume_block(parser, chunk, &bytes)))
                {
                    return error;
                }

                if (chunk->nodes[chunk->nodes_count - 1].kind == NODE_EOF)
                {
                    parser->error_offset = start - parser->start;
                    parser->error_message = "unterminated block";
                    return EINVAL;
                }

                // we are sure the last is ']'

                *out = (ast_node_t) {
                    .kind = NODE_LOOP,
                    .source = start,
                    .chunk = chunk,
                };
                *out_size = real_start - start + bytes;
                return 0;
            }

            if (*parser->ptr == ']')
            {
                parser->error_offset = parser->ptr - parser->start;
                parser->error_message = "unexpected ]";
                return EINVAL;
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
        *out_size = parser->ptr - real_start;
        return 0;
    }

    // end of the node
    *out = (ast_node_t) {
        .kind = current,
        .source = start,
        .operands = operands,
    };
    *out_size = parser->ptr - real_start;
    return 0;
}

int parser_consume_chunk(struct __bf_parser *parser, ast_chunk_t *out, size_t *out_size)
{
    int error;

    // allocating by blocks of 16
    size_t cap = 16, count = 0;
    ast_node_t *list = calloc(cap, sizeof(ast_node_t));

    if (!list)
    {
        return ENOMEM;
    }

    ast_node_t last;
    size_t bytes = 0;

    do
    {
        size_t new_bytes;
        if ((error = parser_consume_node(parser, &last, &new_bytes)))
        {
            free(list);
            return error;
        }
        bytes += new_bytes;

        if (cap == count)
        {
            cap += 16;
            ast_node_t *new_list = realloc(list, cap * sizeof(ast_node_t));

            if (!new_list)
            {
                free(list);
                return ENOMEM;
            }

            list = new_list;
        }

        list[count++] = last;
    }
    while (last.kind != NODE_EOF);

    *out = (ast_chunk_t) {
        .nodes_count = count,
        .nodes = list,
    };
    *out_size = bytes;
    return 0;
}

int parser_consume_block(struct __bf_parser *parser, ast_chunk_t *out, size_t *out_size)
{
    int error;

    size_t cap, count = 0;
    ast_node_t *list = calloc(cap = 16, sizeof(ast_node_t));

    if (!list)
    {
        return ENOMEM;
    }

    ast_node_t last;
    size_t bytes = 0;

    while (parser->ptr - parser->start <= parser->len)
    {
        size_t new_bytes;
        parser_consume_comment(parser, &new_bytes);
        bytes += new_bytes;

        if (parser->ptr - parser->start > parser->len)
        {
            break;
        }

        if (*parser->ptr == ']')
        {
            bytes += 1;
            parser->ptr++;
            break;
        }

        if ((error = parser_consume_node(parser, &last, &new_bytes)))
        {
            free(list);
            return error;
        }

        bytes += new_bytes;

        if (cap == count)
        {
            cap += 16;
            ast_node_t *new_list = realloc(list, cap * sizeof(ast_node_t));

            if (!new_list)
            {
                free(list);
                return ENOMEM;
            }

            list = new_list;
        }

        list[count++] = last;
    }

    if (*parser->ptr == 0)
    {
        if (cap == count)
        {
            cap += 16;
            ast_node_t *new_list = realloc(list, cap * sizeof(ast_node_t));

            if (!new_list)
            {
                free(list);
                return ENOMEM;
            }

            list = new_list;
        }

        list[count++] = (ast_node_t) { .kind = NODE_EOF, .source = parser->ptr };
    }

    *out = (ast_chunk_t) {
        .nodes_count = count,
        .nodes = list,
    };
    *out_size = bytes;
    return 0;
}

int bf_prepare_bf(bf_state_t state, const char *code, bf_code_t *out)
{
    return bf_prepares_bf(state, code, strlen(code), out);
}

int bf_prepares_bf(bf_state_t _s, const char *code, size_t size, bf_code_t *out)
{
    struct __bf_parser parser;
    parser_new(&parser, code, size);

    ast_chunk_t *chunk = malloc(sizeof(ast_chunk_t));

    if (!chunk)
    {
        return ENOMEM;
    }

    __code *node = malloc(sizeof(__code));

    if (!node)
    {
        free(chunk);
        return ENOMEM;
    }

    // we do not early return the error, we let the caller access the error message.
    size_t consumed_size;
    int error = parser_consume_chunk(&parser, chunk, &consumed_size);

    *node = (__code) {
        .chunk = chunk,
        .error_offset = parser.error_offset,
        .error_message = parser.error_message,
        .previous = NULL,
        .next = NULL,
    };

    __state *state = _s;
    __code *ring = state->ring;

    state->ring = node;

    if (ring)
    {
        node->next = ring->next;
        node->previous = ring->previous;
        ring->previous = node;
    }
    else
    {
        node->next = node;
        node->previous = node;
    }

    *out = node;
    return error;
}
