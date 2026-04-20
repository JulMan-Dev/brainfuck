#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "parser.h"
#include "libbf/int.h"
#include "__int_state.h"

int bf_eval(bf_state_t _s, char *code)
{
    return bf_evals(_s, code, strlen(code));
}

int bf_evals(bf_state_t _s, char *code, size_t len)
{
    int error;
    parser_t parser;
    ast_chunk_t *chunk;

    __state *state = _s;

    if ((error = __bf_check_init(state)))
    {
        return error;
    }

    if (state->current_frame)
    {
        return EBUSY;
    }

    parser_new(&parser, code, len); // cannot fail

    chunk = malloc(sizeof(ast_chunk_t));

    if (!chunk)
    {
        return ENOMEM;
    }

    parser_consume_chunk(&parser, chunk);

    __frame *frame = malloc(sizeof(__frame));

    if (!frame)
    {
        ast_free(chunk);
        free(chunk);
        return ENOMEM;
    }

    *frame = (__frame) {
        .parent = NULL,
        .kind = frame_kind_chunk,
        .chunk = chunk,
        .pc = 0,
    };
    state->current_frame = frame;
    struct strip_t *strip = &state->strip;

    for (;;)
    {
        frame = state->current_frame;

        if (frame->pc >= frame->chunk->nodes_count)
        {
            if (frame->parent)
            {
                if (strip->buf[strip->index])
                {
                    frame->pc = 0;
                }
                else
                {
                    state->current_frame = frame->parent;
                    state->current_frame->pc++;
                    free(frame);
                }

                continue;
            }

            break; // hell yeah!
        }

        ast_node_t *node = &frame->chunk->nodes[frame->pc];

        switch (node->kind)
        {
        case NODE_EOF:
            // we could also break here, but too lazy to.
            break;

        case NODE_RIGHT:
            if ((error = brainfuck_rt_rig(strip, node->operands)))
            {
                return error;
            }
            break;

        case NODE_LEFT:
            if ((error = brainfuck_rt_lef(strip, node->operands)))
            {
                return error;
            }
            break;

        case NODE_PLUS:
            if ((error = brainfuck_rt_inc(strip, node->operands)))
            {
                return error;
            }
            break;

        case NODE_MINUS:
            if ((error = brainfuck_rt_dec(strip, node->operands)))
            {
                return error;
            }
            break;

        case NODE_OUTPUT:
            if ((error = brainfuck_rt_out(strip, node->operands)))
            {
                return error;
            }
            break;

        case NODE_INPUT:
            if ((error = brainfuck_rt_inp(strip, node->operands)))
            {
                return error;
            }
            break;

        case NODE_LOOP:
            {
                if (!strip->buf[strip->index])
                {
                    break;
                }

                __frame *new_frame = malloc(sizeof(__frame));

                if (!new_frame)
                {
                    return ENOMEM;
                }

                *new_frame = (__frame) {
                    .parent = frame,
                    .kind = frame_kind_chunk,
                    .chunk = node->chunk,
                    .pc = 0,
                };
                state->current_frame = new_frame;
                goto loop_flow;
            }
            break;
        }

        frame->pc++;
loop_flow:
        // noop.
        continue;
    }

    // cleaning things up.
    ast_free(chunk);
    free(chunk);

    while (state->current_frame)
    {
        __frame *older_frame = state->current_frame->parent;
        free(state->current_frame);
        state->current_frame = older_frame;
    }

    return 0;
}

int bf_evalc(bf_state_t _s, bf_block_t block)
{
    int error;
    __state *state = _s;

    if ((error = __bf_check_init(state)))
    {
        return error;
    }

    if (state->current_frame)
    {
        return EBUSY;
    }

    __frame *frame = malloc(sizeof(__frame));

    if (!frame)
    {
        return ENOMEM;
    }

    *frame = (__frame) {
        .parent = NULL,
        .kind = frame_kind_function,
        .block = block,
        .pc = 0,
    };
    state->current_frame = frame;

    struct strip_t *strip = &state->strip;

    error = block(strip);
    free(frame);
    state->current_frame = NULL;
    return error;
}
