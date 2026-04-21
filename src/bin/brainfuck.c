#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "libbf/int.h"

int main(int argc, const char **argv)
{
    int error = 0;

    if (argc == 1)
    {
print_usage:
        fprintf(stderr, "%s [-i <input>] [-o <output>] [-jit] <input.bf>\n", argv[0]);
        return 1;
    }

    int jit = 0;
    FILE *input = stdin, *output = stdout, *file = NULL;

    for (size_t i = 1; i < argc; i++)
    {
        const char *arg = argv[i];

        if (strcmp(arg, "-jit") == 0)
        {
            jit = 1;
            continue;
        }

        if (strcmp(arg, "-i") == 0)
        {
            if (++i == argc)
            {
                fprintf(stderr, "%s: expected file after -i\n", argv[0]);
                error = 1;
                goto close_files;
            }

            if (input && input != stdin)
            {
                fclose(input);
            }

            input = fopen(argv[i], "r");

            if (!input)
            {
                error = errno;
                fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(error));
                goto close_files;
            }

            continue;
        }

        if (strcmp(arg, "-o") == 0)
        {
            if (++i == argc)
            {
                fprintf(stderr, "%s: expected file after -o\n", argv[0]);
                error = 1;
                goto close_files;
            }

            if (output && output != stdout)
            {
                fclose(output);
            }

            output = fopen(argv[i], "w");

            if (!output)
            {
                error = errno;
                fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(error));
                goto close_files;
            }

            continue;
        }

        if (file)
        {
            fprintf(stderr, "%s: multiple scripts given\n", argv[0]);
            error = 1;
            goto close_files;
        }

        file = fopen(arg, "r");

        if (!file)
        {
            error = errno;
            fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(error));
            goto close_files;
        }
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

    bf_state_t state;

    if ((error = bf_new(&state)))
    {
        fprintf(stderr, "Failed to load runtime: %s\n", strerror(error));
        fclose(file);
        free(buf);
        return 1;
    }

    bf_set_input(state, input);
    bf_set_output(state, output);
    setbuf(output, NULL);

    if (jit)
    {
        error = bf_evals_jit(state, buf, size);
    }
    else
    {
        error = bf_evals(state, buf, size);
    }

    bf_deinit(state);

free_buf:
    free(buf);

close_files:
    if (input && input != stdin)
    {
        fclose(input);
    }

    if (output && output != stdout)
    {
        fclose(output);
    }

    if (file)
    {
        fclose(file);
    }

    return error;
}
