#include <stdlib.h>
#include <sys/errno.h>

#include "libbf/int.h"
#include "__int_state.h"

int bf_new(bf_state_t *out)
{
    int error;

    __state *state = malloc(sizeof(__state));
    if (!state)
    {
        return ENOMEM;
    }

    struct strip_t strip;

    if ((error = brainfuck_rt_init(&strip)))
    {
        free(state);
        return error;
    }

    *state = (__state) {
        .strip = strip,
        .current_frame = NULL,
        .input = stdin,
        .output = stdout,
    };
    *out = state;
    return 0;
}

int bf_deinit(bf_state_t _s)
{
    __state *state = _s;

    __frame *frame = state->current_frame;
    while (frame)
    {
        // ReSharper disable once CppDFADeletedPointer
        frame = frame->parent;

        if (frame->kind == frame_kind_chunk)
        {
            fprintf(stderr, "TODO: free of ast chunk.\n");
            abort();
        }

        free(frame);
    }

    // runtime strip, using libbf_rt
    brainfuck_rt_deinit(&state->strip);

    free(state);
    return 0;
}

int bf_set_input(bf_state_t _s, FILE *input)
{
    __state *state = _s;

    state->input = input;
    brainfuck_rt_ext_sinp(&state->strip, input);
    return 0;
}

int bf_set_output(bf_state_t _s, FILE *output)
{
    __state *state = _s;

    state->output = output;
    brainfuck_rt_ext_sout(&state->strip, output);
    return 0;
}

int __bf_check_init(__state *state)
{
    return 0;
}
