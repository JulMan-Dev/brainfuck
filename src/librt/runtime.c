#include "libbf/rt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

API_HIDDEN int __allocate_strip(unsigned char **strip, size_t *cap)
{
    unsigned char *new_strip;

    size_t old_cap = *cap;
    *cap += 16;

    new_strip = realloc(*strip, *cap);

    if (!new_strip)
    {
        if (*strip)
        {
            // freeing old buffer
            free(*strip);
            *strip = NULL;
        }

        return ENOMEM;
    }

    memset(new_strip + old_cap, 0, *cap - old_cap);

    *strip = new_strip;
    return 0;
}

int brainfuck_rt_init(struct strip_t *out)
{
    unsigned char *buf = NULL;
    size_t cap = 0;

    setbuf(stdout, NULL);

    int ret = __allocate_strip(&buf, &cap);
    if (ret)
    {
        fprintf(stderr, "fatal error: cannot init brainfuck runtime: %s", strerror(ret));
        return ret;
    }

    *out = (struct strip_t) {
        .buf = buf,
        .cap = cap,
        .index = 0,
        .err = 0,
    };
    return 0;
}

int brainfuck_rt_deinit(struct strip_t *strip)
{
    setbuf(stdout, "\n");

    if (strip->buf != NULL)
    {
        free(strip->buf);
    }

    strip->cap = 0;
    strip->index = 0;
    strip->err = 0;
    return 0;
}

int brainfuck_rt_lef(struct strip_t *strip, size_t operand)
{
    if (operand > strip->index)
    {
        brainfuck_rt_error(strip, EFAULT);
        brainfuck_rt_deinit(strip);
        return EFAULT;
    }

    strip->index -= operand;
    return 0;
}

int brainfuck_rt_rig(struct strip_t *strip, size_t operand)
{
    size_t new_index = strip->index + operand;

    while (strip->cap <= new_index)
    {
        int ret = __allocate_strip(&strip->buf, &strip->cap);
        if (ret)
        {
            brainfuck_rt_error(strip, ret);
            brainfuck_rt_deinit(strip);
            return ret;
        }
    }

    strip->index += operand;
    return 0;
}

int brainfuck_rt_inc(struct strip_t *strip, size_t operand)
{
    strip->buf[strip->index] += operand;
    return 0;
}

int brainfuck_rt_dec(struct strip_t *strip, size_t operand)
{
    strip->buf[strip->index] -= operand;
    return 0;
}

int brainfuck_rt_out(struct strip_t *strip, size_t operand)
{
    int ret;
    unsigned char value = strip->buf[strip->index];

    for (size_t i = 0; i < operand; i++)
    {
        ret = fputc(value, stdout);

        if (ret == EOF && ferror(stdout))
        {
            brainfuck_rt_error(strip, EOF);
            brainfuck_rt_deinit(strip);
            return EOF;
        }
    }

    return 0;
}

int brainfuck_rt_inp(struct strip_t *strip, size_t operand)
{
    int ret;

    for (size_t i = 0; i < operand; i++)
    {
        ret = fgetc(stdin);

        if (ret == EOF)
        {
            // silent EOF, 0 instead
            if (feof(stdin))
            {
                ret = 0;
            }
            else if (ferror(stdin))
            {
                brainfuck_rt_error(strip, EIO);
                brainfuck_rt_deinit(strip);
                return EIO;
            }
        }

        strip->buf[strip->index] = ret;
    }

    return 0;
}

int brainfuck_rt_loop(struct strip_t *strip, bf_block_t block)
{
    int ret;

    while (strip->buf[strip->index])
    {
        ret = block(strip);

        if (ret)
        {
            // we do not re emit from here.
            return ret;
        }
    }

    return 0;
}

int brainfuck_rt_error(struct strip_t *strip, int err)
{
    fprintf(stderr, "brainfuck runtime error: %s\n", strerror(err));
    return 0;
}
