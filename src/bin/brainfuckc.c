#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "parser.h"
#include "ast.h"
#include "codegen.h"

int main(int argc, const char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "%s <input.bf> <output.c>\n", argv[0]);
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
    parser_new(&parser, buf, size);

    ast_chunk_t chunk;
    parser_consume_chunk(&parser, &chunk);

    FILE *output = fopen(argv[2], "w");
    if (!output)
    {
        fprintf(stderr, "%s: %s: %s\n", argv[0], argv[2], strerror(errno));
        fclose(file);
        free(buf);
        return 1;
    }

    codegen_t generator;
    codegen_new(&generator, &chunk);
    codegen_generate(&generator, NULL);
    codegen_flush(generator, output);

    fclose(file);
    free(buf);
    fclose(output);
    return 0;
}
