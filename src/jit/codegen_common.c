#include <stdlib.h>
#include <string.h>

#include "jit.h"

int jit_generator_new(jit_codegen_t *out, lib_symbols_t *symbols, ast_chunk_t *chunk)
{
    *out = (jit_codegen_t) {
        .chunk = chunk,
        .generated = NULL,
        .cap = 0,
        .len = 0,
        .patches = NULL,
        .pcap = 0,
        .plen = 0,
        .symbols = symbols,
    };
    return 0;
}

size_t jit_generator_code_len(jit_codegen_t const *generator)
{
    return generator->len;
}

int jit_generator_flush(jit_codegen_t *generator, unsigned char *out, size_t cap)
{
    int error;

    if ((error = jit_generator_patch(generator, out)))
    {
        return error;
    }

    memcpy(out, generator->generated, cap < generator->len ? cap : generator->len);
    return 0;
}

int jit_generator_free(jit_codegen_t const *generator)
{
    if (generator->generated)
    {
        free(generator->generated);
    }

    if (generator->patches)
    {
        free(generator->patches);
    }

    return 0;
}
