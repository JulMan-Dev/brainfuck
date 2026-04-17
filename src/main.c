#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>

#include "parser.h"
#include "ast.h"

int main(int argc, const char **argv) {
    FILE *file = fopen(argv[1], "r");
    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buf = malloc(size);
    if (fread(buf, 1, size, file) < size)
    {
        printf("Failed to read %s.\n", argv[1]);
        return 1;
    }

    parser_t parser;
    parser_new(&parser, buf);

    ast_chunk_t chunk;
    size_t bytes = parser_consume_chunk(&parser, &chunk);

    ast_inspect_chunk(&chunk);
    printf("consumed %lu bytes\n", bytes);

    free(buf);
    return 0;
}
