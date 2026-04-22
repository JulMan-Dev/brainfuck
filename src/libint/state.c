#include <dlfcn.h>
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

    void *handle = dlopen(NULL, RTLD_NOW);

    lib_symbols_t symbols = (lib_symbols_t) {
        .handle = handle,
        .left = dlsym(handle, "brainfuck_rt_lef"),
        .right = dlsym(handle, "brainfuck_rt_rig"),
        .inc = dlsym(handle, "brainfuck_rt_inc"),
        .dec = dlsym(handle, "brainfuck_rt_dec"),
        .out = dlsym(handle, "brainfuck_rt_out"),
        .inp = dlsym(handle, "brainfuck_rt_inp"),
        .loop = dlsym(handle, "brainfuck_rt_loop"),
    };

    *state = (__state) {
        .strip = strip,
        .current_frame = NULL,
        .input = stdin,
        .output = stdout,
        .symbols = symbols,
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
            ast_free(frame->chunk);
        }

        free(frame);
    }

    // runtime strip, using libbf_rt
    brainfuck_rt_deinit(&state->strip);

    dlclose(state->symbols.handle);

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
