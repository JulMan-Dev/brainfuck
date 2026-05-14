#include <stdlib.h>
#include <sys/errno.h>

#include "jit.h"

int jit_generator_prepare(jit_codegen_t *generator)
{
    fprintf(stderr, "JIT: prepare: Unsupported current architecture\n");
    return ENOTSUP;
}

int jit_generator_patch(jit_codegen_t *generator, unsigned char *ptr)
{
    fprintf(stderr, "JIT: flush: Unsupported current architecture\n");
    return ENOTSUP;
}
