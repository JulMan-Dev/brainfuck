#include <string.h>
#include <sys/errno.h>

#include "libbf/int.h"
#include "__int_state.h"

int bf_get(bf_state_t _s, unsigned char *out, size_t index)
{
    int error;
    __state *state = _s;

    if ((error = __bf_check_init(state)))
    {
        return error;
    }

    struct strip_t *strip = &state->strip;

    if (index >= strip->cap)
    {
        return EFAULT;
    }

    if (out)
    {
        *out = strip->buf[index];
    }

    return 0;
}

int bf_read(bf_state_t _s, unsigned char *out, size_t start_index, size_t length)
{
    int error;
    __state *state = _s;

    if ((error = __bf_check_init(state)))
    {
        return error;
    }

    struct strip_t *strip = &state->strip;

    if (!length)
    {
        return EINVAL;
    }

    if (start_index >= strip->cap || start_index + length >= strip->cap)
    {
        return EFAULT;
    }

    if (out)
    {
        memcpy(out, strip->buf + start_index, length);
    }

    return 0;
}
