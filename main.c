#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "encode.h"
#include "decode.h"

void print_version()
{
    printf("Huffman Test 0.1\n");
}

void print_usage()
{
    printf("\e[1mUsage:\e[0m\n");
    printf("        huffman option [infile [outfile]]\n");
    printf("\e[1mOptions:\e[0m\n");
    printf("    -e      encode\n");
    printf("    -d      decode\n");
    printf("    -v      version\n");
}

enum Direction {
    ENCODE,
    DECODE
};

int main(int argc, char **argv)
{
    if (argc < 2 || 4 < argc) {
        print_usage();
        return EXIT_FAILURE;
    }

    // Handle command line options
    if (strcmp(argv[1], "-v") == 0) {
        print_version();
        return EXIT_SUCCESS;
    }
    enum Direction dir;
    if (strcmp(argv[1], "-e") == 0) {
        dir = ENCODE;
    } else if (strcmp(argv[1], "-d") == 0) {
        dir = DECODE;
    } else {
        print_usage();
        return EXIT_FAILURE;
    }

    FILE *fp;
    size_t ret;
    size_t buffer_size, result_size;
    uint8_t *buffer, *result;
    bool flag;

    // Open input file for reading.
    if (argc >= 3) {
        fp = fopen(argv[2], "rb");
        if (!fp) {
            perror("fopen");
            return EXIT_FAILURE;
        }
    } else {
        fp = freopen(NULL, "rb", stdin);
        if (!fp) {
            perror("freopen");
            return EXIT_FAILURE;
        }
    }

    // Read from input file.
    flag = false;
    buffer_size = 1024;
    buffer = (uint8_t *) malloc(buffer_size * sizeof(*buffer));
    ret = fread(buffer, sizeof(*buffer), buffer_size, fp);
    while (ret > 0) {
        buffer = (uint8_t*) realloc(buffer, 2 * buffer_size * sizeof(*buffer));
        ret = fread(buffer + buffer_size, sizeof(*buffer), buffer_size, fp);
        buffer_size += ret;
        if (ferror(fp)) {
            flag = true;
            break;
        }
        if (feof(fp)) {
            break;
        }
    }
    fclose(fp);
    if (flag) {
        goto fail1;
    }

    if (dir == ENCODE) {
        result_size = encode(buffer, buffer_size, &result);
    } else {
        result_size = decode(buffer, buffer_size, &result);
    }
    free(buffer);

    // Open output file for writing.
    if (argc >= 4) {
        fp = fopen(argv[3], "wb");
        if (!fp) {
            perror("fopen");
            goto fail2;
        }
    } else {
        fp = freopen(NULL, "wb", stdout);
        if (!fp) {
            perror("freopen");
            goto fail2;
        }
    }

    // Write to, and close output file.
    ret = fwrite(result, sizeof(*result), result_size, fp);
    fclose(fp);
    if (ret < result_size) {
        perror("fwrite");
        goto fail2;
    }

success:
    free(result);
    return EXIT_SUCCESS;

fail1:
    free(buffer);
    return EXIT_FAILURE;

fail2:
    free(result);
    return EXIT_FAILURE;
}
