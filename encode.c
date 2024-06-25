#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "encode.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

size_t cnt[512], p[512], l[512], r[512], cur;

void build_tree(uint8_t *buffer, size_t n)
{
    memset(cnt, 0, sizeof(cnt));
    for (size_t i = 0; i < n; i++) {
        cnt[buffer[i]]++;
    }

    for (size_t i = 0; i < 512; i++)
    {
        p[i] = i;
        l[i] = 512;
        r[i] = 512;
    }

    cur = 256;
    while (true) {
        size_t a = 512, b = 512, cnta = SIZE_MAX, cntb = SIZE_MAX;
        for (size_t i = 0; i < cur; i++)
            if (p[i] == i) {
                if (cnt[i] < cntb) {
                    cnta = cntb;
                    a = b;
                    cntb = cnt[i];
                    b = i;
                } else if (cnt[i] < cnta) {
                    cnta = cnt[i];
                    a = i;
                }
            }
        if (a == 512 || b == 512)
            break;
        l[cur] = MIN(a, b);
        r[cur] = MAX(a, b);
        p[a] = cur;
        p[b] = cur;
        cnt[cur] = cnt[a] + cnt[b];
        cur++;
    }
    assert(cur == 511);
    // Though the values of p are between 0 and 511, we know that
    // the set of leaves are 0 to 255, thus we know that all the values
    // of p are between 256 and 511.
    // thus to store our tree we only store 
    // the smallest 8 bits of p[i] for each i = 0..510 :)
    // TODO: use a heap lmao
}

int8_t depth[512]; 
size_t path[512];

void dfs(size_t node)
{
    if (l[node] < 512) {
        path[l[node]] = path[node] << 1;
        depth[l[node]] = depth[node] + 1;
        dfs(l[node]);
    }
    if (r[node] < 512) {
        path[r[node]] = (path[node] << 1) | 1;
        depth[r[node]] = depth[node] + 1;
        dfs(r[node]);
    }
}

size_t reverse(size_t byte, int8_t width)
{
    size_t result = 0;
    for (int8_t l = 0, r = width - 1; l <= r; l++, r--) {
        result |= (((byte >> l) & 1) << r);
        result |= (((byte >> r) & 1) << l);
    }
    return result;
}

size_t encode(uint8_t *buffer, size_t n, uint8_t **result) 
{
    build_tree(buffer, n);

    depth[cur-1] = 0;
    path[cur-1] = 0;
    dfs(cur-1);

    for (size_t i = 0; i < 512; i++)
        path[i] = reverse(path[i], depth[i]);
    
    size_t sz = n;
    *result = (uint8_t *) malloc(n * sizeof(**result));

    uint16_t byte_buf;
    uint8_t offset = 0;
    size_t num_bytes = 512;
    for (size_t i = 0; i < n; i++) {
        byte_buf |= path[buffer[i]] << offset;
        offset += depth[buffer[i]];
        if (offset >= 8) {
            if (num_bytes >= sz) {
                *result = realloc(*result, 2 * sz); 
                sz *= 2;
            }
            (*result)[num_bytes++] = (uint8_t) byte_buf;
            byte_buf >>= 8;
            offset -= 8;
        }
    }
    (*result)[511] = offset;
    if (num_bytes >= sz) {
        *result = realloc(*result, 2 * sz); 
        sz *= 2;
    }
    (*result)[num_bytes++] = (uint8_t) byte_buf;

    for (size_t i = 0; i < 511; i++)
        (*result)[i] = (uint8_t) p[i];

    return num_bytes;
}
