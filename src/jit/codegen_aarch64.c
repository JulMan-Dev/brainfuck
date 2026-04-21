#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "jit.h"

typedef struct __queue __queue;

struct __queue
{
    __queue *parent;
    ast_chunk_t *chunk;
    size_t pc;
};

typedef struct
{
    uint32_t *code;
    size_t cap, len;
    ast_chunk_t *chunk;
} __pending;

#define A64_SP 31
#define A64_XZR 31

// mov Xd, Xn
#define A64_MOV_REG(Xd, Xn) (0xaa0003e0 | (((Xn) & 31) << 16) | (((Xd) & 31)))
// movz Xd, #imm, lsl #shift
#define A64_MOVZ(Xd, imm, shift) (0xd2800000 | ((Xd) & 31) | (((imm) & 0xffff) << 5) | (((shift) / 16) << 21))
// ldp Xt1, Xt2, [Xn], #imm
#define A64_LDP_POST(Xt1, Xt2, Xn, imm) (0xa8c00000 | ((Xt1) & 31) |  (((Xt2) & 31) << 10) | \
    (((Xn) & 31) << 5) | ((((uint32_t) (imm) / 8) & 127) << 15))
// stp Xt1, Xt2, [Xn, #imm]!
#define A64_STP_PRE(Xt1, Xt2, Xn, imm) (0xa9800000 | ((Xt1) & 31) |  (((Xt2) & 31) << 10) | \
    (((Xn) & 31) << 5) | ((((uint32_t) (imm) / 8) & 127) << 15))
// bl #imm
#define A64_BL(imm) (0x94000000 | (((imm) / 4) & 0x3ffffff))
// b #imm
#define A64_B(imm) (0x14000000 | (((imm) / 4) & 0x3ffffff))
// cbnz Xt, #imm
#define A64_CBNZ(Xt, imm) (0xb5000000 | (Xt) & 31 | (((imm) / 4 & 0x7ffff) << 5))
// adr x0, #imm
#define A64_ADR(Xd, imm) (0x10000000 | (((imm) & 0x3) << 29) | (((imm) >> 2 & 0x7ffff) << 5) | ((Xd) & 31))
// ret Xr
#define A64_RET(Xr) (0xd65f0000 | (((Xr) & 31) << 5))

#define PRE_APPEND_INSTRUCTIONS 2

API_HIDDEN int __jit_push_patch(jit_codegen_t *, size_t, void *, uint64_t);

int jit_generator_prepare(jit_codegen_t *generator)
{
    int error = 0;
    size_t chunks_cap = 16, chunks = 0;
    ast_chunk_t **chunks_buf = calloc(chunks_cap, sizeof(ast_chunk_t *));

    if (!chunks_buf)
    {
        return ENOMEM;
    }

    __queue *queue = malloc(sizeof(__queue));

    if (!queue)
    {
        error = ENOMEM;
        goto free_chunks_buf;
    }

    *queue = (__queue) {
        .parent = NULL,
        .chunk = generator->chunk,
        .pc = 0,
    };

queue_start:
    if (chunks == chunks_cap)
    {
        chunks_cap *= 2; // we do exponential here to avoid too many reallocations
        ast_chunk_t **new_buf = realloc(chunks_buf, chunks_cap * sizeof(ast_chunk_t *));

        if (!new_buf)
        {
            error = ENOMEM;
            goto free_queue;
        }

        chunks_buf = new_buf;
    }

    chunks_buf[chunks++] = queue->chunk;

    for (;;)
    {
        ast_chunk_t *chunk = queue->chunk;

        if (queue->pc == chunk->nodes_count)
        {
            // popping to the next
            __queue *current = queue;
            queue = current->parent;
            free(current);

            if (queue)
            {
                queue->pc++;
                continue; // we won't goto queue_start because it will push again the already visited chunk
            }

            break;
        }

        if (chunk->nodes[queue->pc].kind != NODE_LOOP)
        {
            queue->pc++;
            continue;
        }

        __queue *next = malloc(sizeof(__queue));

        if (!next)
        {
            error = ENOMEM;
            goto free_queue;
        }

        *next = (__queue) {
            .parent = queue,
            .chunk = chunk->nodes[queue->pc].chunk,
            .pc = 0,
        };
        queue = next;
        goto queue_start;
    }

    __pending *generated_chunks = calloc(chunks, sizeof(__pending));

    if (!generated_chunks)
    {
        error = ENOMEM;
        goto free_queue;
    }

    size_t total = PRE_APPEND_INSTRUCTIONS;

    for (size_t j = 0; j < chunks; j++)
    {
        __pending *pending = &generated_chunks[j];
        pending->chunk = chunks_buf[chunks - 1 - j];

        size_t instructions = pending->chunk->nodes_count;

        uint32_t *code = calloc(instructions * 4 + 7, sizeof(uint32_t));

        if (!code)
        {
            error = ENOMEM;
            goto free_generated_chunks;
        }

        pending->code = code;
        pending->cap = instructions * 4 + 7;
        pending->len = 0;

        size_t *l = &pending->len;

#define PUSH_CODE code[(*l)++] =

        // stp x29, x30, [sp, #-0x10]!
        PUSH_CODE A64_STP_PRE(29, 30, A64_SP, -0x10);
        // stp x19, x20, [sp, #-0x10]!
        PUSH_CODE A64_STP_PRE(19, 20, A64_SP, -0x10);
        // mov x19, x0
        PUSH_CODE A64_MOV_REG(19, 0);

        for (size_t k = 0; k < pending->chunk->nodes_count; k++)
        {
            ast_node_t *node = &pending->chunk->nodes[k];
            bf_instruction_t inst;

            switch (node->kind)
            {
            case NODE_EOF:
                break;

            case NODE_RIGHT:
                inst = generator->symbols->right;
                goto code_operand;

            case NODE_LEFT:
                inst = generator->symbols->left;
                goto code_operand;

            case NODE_PLUS:
                inst = generator->symbols->inc;
                goto code_operand;

            case NODE_MINUS:
                inst = generator->symbols->dec;
                goto code_operand;

            case NODE_OUTPUT:
                inst = generator->symbols->out;
                goto code_operand;

            case NODE_INPUT:
                inst = generator->symbols->inp;
code_operand:
                {
                    // mov x0, x19
                    PUSH_CODE A64_MOV_REG(0, 19);
                    // mov x1, #<operands>
                    PUSH_CODE A64_MOVZ(1, node->operands, 0);
                    // bl <instruction>
                    PUSH_CODE A64_BL(0);
                    if ((error = __jit_push_patch(generator, total + *l - 1, inst, 0)))
                    {
                        goto free_generated_chunks;
                    }
                    // cbnz x0, __jit_builtin_error
                    PUSH_CODE A64_CBNZ(0, (-total - *l + 1) * 4);
                    /*if ((error = __jit_push_patch(generator, total + *l - 1, __jit_builtin_error, 1)))
                    {
                        goto free_generated_chunks;
                    }*/
                }
                break;
            case NODE_LOOP:
                {
                    // mov x0, x19
                    PUSH_CODE A64_MOV_REG(0, 19);

                    ast_chunk_t *target = node->chunk;
                    __pending *found = NULL;
                    size_t path = PRE_APPEND_INSTRUCTIONS;

                    for (size_t m = 0; m < chunks; m++) // too many nested indices.
                    {
                        __pending *current = &generated_chunks[m];

                        if (current->chunk == NULL || current->chunk == target)
                        {
                            found = current;
                            break;
                        }

                        path += current->len;
                    }

                    if (!found || !found->chunk)
                    {
                        error = EFAULT;
                        goto free_generated_chunks;
                    }

                    ssize_t imm = path - total - *l ;

                    // adr x1, <imm>
                    PUSH_CODE A64_ADR(1, imm * 4);

                    // bl brainfuck_rt_loop
                    PUSH_CODE A64_BL(0);
                    if ((error = __jit_push_patch(generator, total + *l - 1, generator->symbols->loop, 0)))
                    {
                        goto free_generated_chunks;
                    }

                    // cbnz x0, __jit_builtin_error
                    ssize_t M = -total - *l + 1;
                    PUSH_CODE A64_CBNZ(0, M * 4);
                    /*if ((error = __jit_push_patch(generator, total + *l - 1, __jit_builtin_error, 1)))
                    {
                        goto free_generated_chunks;
                    }*/
                }
                break;
            }
        }

        // mov x0, xzr
        PUSH_CODE A64_MOV_REG(0, A64_XZR);
        // ldp x19, x20, [sp], #0x10
        PUSH_CODE A64_LDP_POST(19, 20, A64_SP, 0x10);
        // ldp x29, x30, [sp], #0x10
        PUSH_CODE A64_LDP_POST(9, 30, A64_SP, 0x10);
        // ret
        PUSH_CODE A64_RET(30);

        total += *l;
    }

    // aggregating all chunks in one piece of code.
    uint32_t *all_code = calloc(total, sizeof(uint32_t));
    size_t current = 1;

    if (!all_code)
    {
        error = ENOMEM;
        goto free_generated_chunks;
    }

    all_code[current++] = A64_RET(30);

    for (size_t i = 0; i < chunks; i++)
    {
        __pending c = generated_chunks[i];
        memcpy(all_code + current, c.code, c.len * sizeof(uint32_t));

        if (i == chunks - 1)
        {
            all_code[0] = A64_B(current * 4);
        }

        current += c.len;
    }

    generator->generated = (unsigned char *)all_code;
    generator->len = current * sizeof(uint32_t);
    generator->cap = total * sizeof(uint32_t);

free_generated_chunks:
    for (size_t i = 0; i < chunks; i++)
    {
        if (generated_chunks[i].code)
        {
            free(generated_chunks[i].code);
        }
    }

free_queue:
    // free remaining queue
    while (queue)
    {
        __queue *next = queue->parent;
        free(queue);
        queue = next;
    }

free_chunks_buf:
    free(chunks_buf);

    return error;
}

int __jit_push_patch(jit_codegen_t *generator, size_t rel, void *abs, uint64_t data)
{
    if (generator->pcap == generator->plen)
    {
        if (!generator->pcap)
        {
            generator->pcap = 16;
        }
        else
        {
            generator->pcap *= 2;
        }

        patch_t *new_patches = realloc(generator->patches, generator->pcap * sizeof(patch_t));

        if (!new_patches)
        {
            return ENOMEM;
        }

        generator->patches = new_patches;
    }

    generator->patches[generator->plen++] = (patch_t) {
        .rel = rel,
        .abs = abs,
        .data = data,
    };

    return 0;
}

int jit_generator_patch(jit_codegen_t *generator, unsigned char *ptr)
{
    for (size_t i = 0; i < generator->plen; i++)
    {
        patch_t *patch = &generator->patches[i];

        if (patch->data > 1)
        {
            return EINVAL;
        }

        ssize_t imm = (ptrdiff_t)patch->abs - (ptrdiff_t)(ptr + patch->rel * sizeof(uint32_t));
        uint32_t *code = (uint32_t *)generator->generated;

        if (patch->data == 0) // bl
        {
            code[patch->rel] = A64_BL(imm);
        }
        else // cbnz
        {
            code[patch->rel] = A64_CBNZ(0, imm);
        }
    }

    return 0;
}
