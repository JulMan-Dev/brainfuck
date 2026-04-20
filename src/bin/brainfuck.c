#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "libbf/int.h"

int main(int argc, const char **argv)
{
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
    if (fread(buf, 1, size, file) < size)
    {
        fprintf(stderr, "Failed to read %s.\n", argv[1]);
        fclose(file);
        free(buf);
        return 1;
    }
    buf[size] = 0;

    int error;
    bf_state_t state;

    if ((error = bf_new(&state)))
    {
        fprintf(stderr, "Failed to load runtime: %s\n", strerror(error));
        fclose(file);
        free(buf);
        return 1;
    }

    if (bf_evals(state, buf, size))
    {
        bf_deinit(state);
        fclose(file);
        free(buf);
        return 1;
    }

    bf_deinit(state);
    fclose(file);
    free(buf);
    return 0;
}
