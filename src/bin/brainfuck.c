#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "parser.h"
#include "ast.h"
#include "runtime.h"

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

    parser_t parser;
    parser_new(&parser, buf);

    ast_chunk_t chunk;
    parser_consume_chunk(&parser, &chunk);

    state_t state;
    state_new(&state, &chunk);

    while (state_can_step(&state))
    {
        if (!state_step(&state))
        {
            int err = state.current_frame->err;
            fprintf(stderr, "Runtime error: %s\n", strerror(err));
            fclose(file);
            free(buf);
            return 1;
        }
    }

    fclose(file);
    free(buf);
    return 0;
}
