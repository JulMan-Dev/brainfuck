#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "libbf/int.h"

int main(int argc, const char **argv)
{
    int error = 0;

    int jit = 0;
    FILE *input = stdin, *output = stdout, *file = NULL;
    const char *file_name = NULL;

    if (argc == 1)
    {
print_usage:
        fprintf(stderr, "%s [-i <input>] [-o <output>] [-jit] <input.bf>\n", argv[0]);
        error = 1;
        goto close_files;
    }

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
        file_name = arg;

        if (!file)
        {
            error = errno;
            fprintf(stderr, "%s: %s: %s\n", argv[0], arg, strerror(error));
            goto close_files;
        }
    }

    if (!file)
    {
        goto print_usage;
    }

    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buf = malloc(size + 1);
    if (fread(buf, 1, size, file) < size)
    {
        fprintf(stderr, "%s: %s: %s.\n", argv[0], file_name, strerror(errno));
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

    bf_code_t code = NULL;

    if ((error = bf_prepares_bf(state, buf, size, &code)))
    {
        if (!code)
        {
            // not enough memory, nothing to do
            goto deinit;
        }

        const char *error_message;
        size_t error_offset;

        bf_code_error(state, code, &error_message, &error_offset);

        fprintf(stderr, "syntax error: %s:%zu: %s\n", file_name, error_offset + 1, error_message);
        goto free_code;
    }

    if (jit)
    {
        error = bf_evalp_jit(state, code);
    }
    else
    {
        error = bf_evalp(state, code);
    }

    if (error)
    {
        fprintf(stderr, "runtime error: %s\n", strerror(error));
    }

free_code:
    bf_code_free(state, code);

deinit:
    bf_deinit(state);

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
