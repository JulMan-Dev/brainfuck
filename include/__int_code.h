#pragma once

#include "ast.h"

typedef struct __code __code;

struct __code
{
    ast_chunk_t *chunk;
    size_t error_offset;
    const char *error_message;

    __code *previous, *next;
};
