#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include "decode.h"

size_t decode(uint8_t *buffer, size_t n, uint8_t **result) 
{
    static size_t p[512], l[512], r[512], cur;
    static size_t depth[512], path[512];
    for (size_t i = 0; i < 511; i++) {
        p[i] = 256 + (uint16_t) buffer[i];
        l[i] = r[i] = 512;
    }

    for (size_t i = 509; ~i; i--) {
        if (r[p[i]] == 512) {
            r[p[i]] = i;
        } else if (l[p[i]] == 512) {
            l[p[i]] = i;
        } else {
            assert(false);
        }
    }

    size_t sz = 0, result_size = n;
    *result = (uint8_t *) malloc(result_size);
    cur = 510;
    for (size_t i = 512; i < n - 1; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            if (buffer[i] & (1 << j))
                cur = r[cur];
            else
                cur = l[cur];
            if (l[cur] == 512 || r[cur] == 512) {
                if (sz >= result_size) {
                    *result = realloc(*result, result_size * 2);
                    result_size *= 2;
                }
                (*result)[sz++] = cur;
                cur = 510;
            }
        }
    }
    size_t offset = buffer[511];
    for (size_t j = 0; j < offset; j++)
    {
        if (buffer[n-1] & (1 << j))
            cur = r[cur];
        else
            cur = l[cur];
        if (l[cur] == 512 || r[cur] == 512) {
            if (sz >= result_size) {
                *result = realloc(*result, result_size * 2);
                result_size *= 2;
            }
            (*result)[sz++] = cur;
            cur = 510;
        }
    }
    return sz;
}
