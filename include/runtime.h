#pragma once

#include <stdbool.h>

#include "common.h"
#include "ast.h"

typedef struct frame_t frame_t;

struct frame_t {
    frame_t *parent;
    ast_chunk_t *chunk;
    int err;
    size_t pc;
};

typedef struct
{
    ast_chunk_t *main_chunk;
    frame_t *current_frame;
    unsigned char *strip;
    size_t strip_index,
        strip_len;
} state_t;

API_HIDDEN void frame_new(frame_t *, ast_chunk_t *);
API_HIDDEN void frame_with_parent(frame_t *, frame_t *, ast_chunk_t *);

API_HIDDEN void state_new(state_t *, ast_chunk_t *);
API_HIDDEN bool state_step(state_t *);
API_HIDDEN bool state_eval_node(state_t *, ast_node_t *);
API_HIDDEN bool state_can_step(state_t *);
