#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "libbf/int.h"

int main(int argc, const char **argv)
{
    int error = 0;
    setbuf(stdout, NULL);

    if (argc == 1)
    {
        fprintf(stderr, "%s <input.bf>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
        return 1;
    }

    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buf = malloc(size + 1);

    if (!buf)
    {
        fprintf(stderr, "Failed to read %s: %s\n", argv[1], strerror(ENOMEM));
        error = ENOMEM;
        goto close;
    }

    if (fread(buf, 1, size, file) < size)
    {
        error = errno;
        fprintf(stderr, "Failed to read %s: %s\n", argv[1], strerror(error));
        goto close;
    }
    buf[size] = 0;

    bf_state_t state;

    if ((error = bf_new(&state)))
    {
        fprintf(stderr, "Failed to load runtime: %s\n", strerror(error));
        goto free_buf;
    }

    if ((error = bf_evals_jit(state, buf, size)))
    {
        error = 1;
    }

    bf_deinit(state);
free_buf:
    free(buf);
close:
    fclose(file);
    return error;
}
