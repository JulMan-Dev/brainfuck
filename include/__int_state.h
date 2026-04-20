#pragma once

#include <stdio.h>

#include "libbf/rt.h"
#include "ast.h"

typedef struct __frame __frame;

typedef struct
{
    struct strip_t strip;
    __frame *current_frame;
    FILE *input,
        *output;
} __state;

typedef enum
{
    frame_kind_chunk,
    frame_kind_function,
} frame_kind;

struct __frame
{
    __frame *parent;
    frame_kind kind;
    union
    {
        ast_chunk_t *chunk;
        bf_block_t block;
    };
    size_t pc; // always 0 for frame_kind_function
};

API_HIDDEN int __bf_check_init(__state *);
