#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <unistd.h>

static uint32_t k[] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

static inline uint32_t right_rotate(uint32_t val, int n) {
    return (val >> n) | (val << (32 - n));
}

static inline void decode_uint32(uint32_t *val) {
    *val = ((*val & 0xff) << 24) | ((*val & 0xff00) << 8) |
           ((*val & 0xff0000) >> 8) | ((*val & 0xff000000) >> 24);
}

static inline void write_uint64_be(uint64_t val, void *to) {
    memcpy(to, &val, sizeof(uint64_t));
    for (int i = 0; i < sizeof(uint64_t) / 2; ++i) {
        ((uint8_t *)to)[i] ^= ((uint8_t *)to)[sizeof(uint64_t) - i - 1];
        ((uint8_t *)to)[sizeof(uint64_t) - i - 1] ^= ((uint8_t *)to)[i];
        ((uint8_t *)to)[i] ^= ((uint8_t *)to)[sizeof(uint64_t) - i - 1];
    }
    memcpy(&val, to, sizeof(uint64_t));
}

static void hash_chunk(uint32_t *chunk, uint32_t *hash) {
    uint32_t w[64];

    for (int i = 0; i < 16; ++i) {
        w[i] = chunk[i];
    }

    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = right_rotate(w[i - 15], 7) ^ right_rotate(w[i - 15], 18) ^
                      (w[i - 15] >> 3);
        uint32_t s1 = right_rotate(w[i - 2], 17) ^ right_rotate(w[i - 2], 19) ^
                      (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t h[8];
    for (int i = 0; i < 8; ++i) {
        h[i] = hash[i];
    }

    for (int i = 0; i < 64; ++i) {
        uint32_t s1 = right_rotate(h[4], 6) ^ right_rotate(h[4], 11) ^
                      right_rotate(h[4], 25);
        uint32_t ch = (h[4] & h[5]) ^ ((~h[4]) & h[6]);
        uint32_t temp1 = h[7] + s1 + ch + k[i] + w[i];
        uint32_t s0 = right_rotate(h[0], 2) ^ right_rotate(h[0], 13) ^
                      right_rotate(h[0], 22);
        uint32_t maj = (h[0] & h[1]) ^ (h[0] & h[2]) ^ (h[1] & h[2]);
        uint32_t temp2 = s0 + maj;

        h[7] = h[6];
        h[6] = h[5];
        h[5] = h[4];
        h[4] = h[3] + temp1;
        h[3] = h[2];
        h[2] = h[1];
        h[1] = h[0];
        h[0] = temp1 + temp2;
    }

    for (int i = 0; i < 8; ++i) {
        hash[i] += h[i];
    }
}

/*
 * Parameters:
 * msg:    Byte array to hash.
 * len:    Length of msg.
 * result: Buffer to write result to. Should be length of 32 bytes (256 bits).
 */
void sha256sum(void const *msg, size_t len, uint8_t *result) {
    uint8_t const *message = (uint8_t const *)msg;

    uint32_t *hash = (uint32_t *)(void *)result;
    hash[0] = 0x6a09e667;
    hash[1] = 0xbb67ae85;
    hash[2] = 0x3c6ef372;
    hash[3] = 0xa54ff53a;
    hash[4] = 0x510e527f;
    hash[5] = 0x9b05688c;
    hash[6] = 0x1f83d9ab;
    hash[7] = 0x5be0cd19;

    uint32_t chunk[16]; // 512 = 16 * sizeof(uint32_t)
    size_t i;
    // 64 bytes = 512 bits
    // for each chunk
    for (i = 0;; i += 64) {
        if (len < i + 64) {
            break;
        }
        for (int j = 0; j < 16; ++j) {
            memcpy(&chunk[j], &message[i + j * sizeof(uint32_t)],
                   sizeof(uint32_t));
            decode_uint32(&chunk[j]);
        }
        hash_chunk(chunk, hash);
    }

    // Process unaligned part in the message.

    int ci;
    for (ci = 0;; ++ci) {
        if (len < i + sizeof(uint32_t)) {
            break;
        }

        memcpy(&chunk[ci], &message[i], sizeof(uint32_t));
        decode_uint32(&chunk[ci]);

        i += 4;
    }

    uint8_t last_area[sizeof(uint32_t)];
    memset(last_area, 0, sizeof(uint32_t));
    memcpy(last_area, message + i, len - i);
    // append one '1' bit.
    last_area[len - i] = 0x80;
    for (int j = len - i + 1; j < sizeof(uint32_t); ++j) {
        last_area[j] = 0x0;
    }
    memcpy(&chunk[ci], last_area, sizeof(uint32_t));
    decode_uint32(&chunk[ci]);
    ++ci;

    if (ci > 14) {
        for (int j = ci; j < 16; ++j) {
            chunk[j] = 0x0;
        }
        hash_chunk(chunk, hash);
        ci = 0;
    }

    for (int j = ci; j < 14; ++j) {
        chunk[j] = 0x0;
    }

    write_uint64_be(len * 8, chunk + 14);
    decode_uint32(&chunk[14]);
    decode_uint32(&chunk[15]);
    hash_chunk(chunk, hash);

    for (int i = 0; i < 8; ++i) {
        decode_uint32(&hash[i]);
    }
}

int main(void) {
    uint8_t buf[4096];
    size_t cap = 4096;
    size_t len = 0;
    uint8_t *data = malloc(cap);
    for (;;) {
        ssize_t n = read(0, buf, 4096);
        if (n <= 0) {
            break;
        }

        if (len + n > cap) {
            cap <<= 1;
            data = realloc(data, cap);
        }

        memcpy(data + len, buf, n);
        len += n;
    }

    uint8_t digest[32];
    sha256sum(data, len, digest);

    free(data);

    for (int i = 0; i < 32; ++i) {
        if (i != 0 && i % 4 == 0) {
            printf(" ");
        }
        printf("%02x", digest[i]);
    }
    putchar('\n');
}
