#include "runtime.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>

#undef errno // mandatory

#define MAX(a, b) ((a) > (b) ? (a) : (b))

void frame_new(frame_t* out, ast_chunk_t* chunk)
{
    *out = (frame_t){
        .parent = NULL,
        .chunk = chunk,
        .errno = 0,
        .pc = 0,
    };
}

void frame_with_parent(frame_t* out, frame_t* parent, ast_chunk_t* chunk)
{
    *out = (frame_t){
        .parent = parent,
        .chunk = chunk,
        .errno = 0,
        .pc = 0,
    };
}

/// Returns the pointer to the strip at index. Allocates memory if required.
API_HIDDEN unsigned char* state_get_strip(state_t* state, size_t index)
{
    if (state->strip_len <= index)
    {
        // allocate enough memory for index (minimum 16)
        size_t amount = MAX(16, index - state->strip_len);
        unsigned char* strip = realloc(state->strip, state->strip_len + amount);
        assert(strip);
        state->strip_len += amount;
        state->strip = strip;
    }

    return state->strip + index;
}

void state_new(state_t* out, ast_chunk_t* chunk)
{
    frame_t* main_frame = malloc(sizeof(frame_t));
    frame_new(main_frame, chunk);

    *out = (state_t){
        .main_chunk = chunk,
        .current_frame = main_frame,
        .strip = NULL,
        .strip_index = 0,
        .strip_len = 0,
    };
}

bool state_step(state_t* state)
{
    if (state->current_frame == NULL)
    {
        return false;
    }

    frame_t* frame = state->current_frame;
    size_t pc = frame->pc;
    ast_chunk_t* chunk = frame->chunk;

    if (chunk->nodes_count == pc)
    {
        // end of frame.
        if (frame->parent == NULL)
        {
            free(state->current_frame);
            state->current_frame = NULL;
            return true; // stopping execution now.
        }

        // checking if we should step out of the frame.
        unsigned char value = *state_get_strip(state, state->strip_index);

        if (value == 0)
        {
            // go up.
            state->current_frame = frame->parent;
            state->current_frame->pc++;
            free(frame);
        }
        else
        {
            // rewind to pc = 0.
            state->current_frame->pc = 0;
        }

        return true;
    }

    if (chunk->nodes_count <= pc)
    {
        frame->errno = EFAULT;
        return false;
    }

    ast_node_t* node = &chunk->nodes[pc];

    return state_eval_node(state, node);
}

bool state_eval_node(state_t* state, ast_node_t* node)
{
    switch (node->kind)
    {
    case NODE_EOF:
        // noop
        break;

    case NODE_RIGHT:
        // moves strip index to right
        {
            state->strip_index += node->operands;
        }
        break;

    case NODE_LEFT:
        // moves strip index to left, fails if underflow.
        {
            if (node->operands > state->strip_index)
            {
                state->current_frame->errno = EIO;
                return false;
            }

            state->strip_index -= node->operands;
        }
        break;

    case NODE_PLUS:
        // increments the value on the strip.
        {
            unsigned char* s = state_get_strip(state, state->strip_index);
            *s += node->operands;
        }
        break;

    case NODE_MINUS:
        // decrements the value on the strip.
        {
            unsigned char* s = state_get_strip(state, state->strip_index);
            *s -= node->operands;
        }
        break;

    case NODE_OUTPUT:
        // prints the strip value in stdout using ASCII.
        {
            unsigned char value = *state_get_strip(state, state->strip_index);
            for (size_t i = 0; i < node->operands; i++)
            {
                putchar(value);
            }
        }
        break;

    case NODE_INPUT:
        // reads one char from stdin and writes on the strip.
        {
            unsigned char* s = state_get_strip(state, state->strip_index);

            for (size_t i = 0; i < node->operands; i++)
            {
                int ch = getchar();

                *s = ch == EOF ? 0 : ch;
            }
        }
        break;

    case NODE_LOOP:
        // Stepping into new frame if current strip is non 0.
        {
            unsigned char value = *state_get_strip(state, state->strip_index);

            if (value == 0)
            {
                break;
            }

            frame_t* new = malloc(sizeof(frame_t));
            assert(new);
            frame_with_parent(new, state->current_frame, node->chunk);
            state->current_frame = new;
            return true;
        }
        break;
    }

    state->current_frame->pc++;
    return true;
}

bool state_can_step(state_t* state)
{
    return state->current_frame != NULL;
}
