#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "libbf/int.h"
#include "__int_code.h"
#include "__int_state.h"

int bf_code_free(bf_state_t _s, bf_code_t _c)
{
    __state *state = _s;
    __code *code = _c;

    if (!state->ring)
    {
        return EINVAL;
    }

    int found = 0;
    __code *current_node = state->ring;
    do
    {
        // found!
        if (current_node == code)
        {
            // proper extraction from the ring
            code->next->previous = code->previous;
            code->previous->next = code->next;

            found = 1;
            break;
        }

        current_node = current_node->next;
    }
    while (current_node != state->ring);

    if (!found)
    {
        return EINVAL;
    }

    // if we remove the ring pointer, we need to change to next.
    if (state->ring == code)
    {
        if (code->next == code)
        {
            // no more ring
            state->ring = NULL;
        }
        else
        {
            state->ring = code->next;
        }
    }

    // finally, freeing the node.
    ast_free(code->chunk);
    free(code);

    // note: we assume __code.error_message to be not on heap.
    return 0;
}

int bf_eval(bf_state_t _s, const char *code)
{
    return bf_evals(_s, code, strlen(code));
}

int bf_evals(bf_state_t _s, const char *code, size_t len)
{
    int error = 0;
    bf_code_t code_handle;

    if ((error = bf_prepares_bf(_s, code, len, &code_handle)))
    {
        const char *error_message;
        size_t error_offset;
        bf_code_error(_s, code_handle, &error_message, &error_offset);

        fprintf(stderr, "bf: error in code:%lu: %s\n", error_offset, error_message);
        goto exit;
    }

    if ((error = bf_evalp(_s, code_handle)))
    {
        fprintf(stderr, "bf: runtime error: %s\n", strerror(error));
    }

exit:
    bf_code_free(_s, code_handle);
    return error;
}

int bf_eval_jit(bf_state_t _s, const char *code)
{
    return bf_evals_jit(_s, code, strlen(code));
}

int bf_evals_jit(bf_state_t _s, const char *code, size_t len)
{
    int error = 0;
    bf_code_t code_handle;

    if ((error = bf_prepares_bf(_s, code, len, &code_handle)))
    {
        const char *error_message;
        size_t error_offset;
        bf_code_error(_s, code_handle, &error_message, &error_offset);

        fprintf(stderr, "bf: error in code:%lu: %s\n", error_offset, error_message);
        goto exit;
    }

    if ((error = bf_evalp_jit(_s, code_handle)))
    {
        fprintf(stderr, "bf: runtime error: %s\n", strerror(error));
    }

exit:
        bf_code_free(_s, code_handle);
    return error;
}

int bf_code_error(bf_state_t _s, bf_code_t _c, const char **out_message, size_t *out_offset)
{
    (void)_s;

    __code *code = _c;

    if (code->error_message)
    {
        *out_message = code->error_message;
        *out_offset = code->error_offset;
        return 0;
    }

    return EINVAL;
}
