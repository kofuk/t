#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline int min3(int n1, int n2, int n3) {
    if (n1 < n2) {
        return n1 < n3 ? n1 : n3;
    } else {
        return n2 < n3 ? n2 : n3;
    }
}

#ifdef SHOW_STEP
static inline int argmin3(int n1, int n2, int n3) {
    if (n1 < n2) {
        return n1 < n3 ? 0 : 2;
    } else {
        return n2 < n3 ? 1 : 2;
    }
}
#endif

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("usage: %s <str1> <str2>\n", argv[0]);
        return 1;
    }

    char *str1 = argv[1];
    char *str2 = argv[2];
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
#ifdef SHOW_STEP
    int *d = malloc(sizeof(int) * (len1 + 1) * (len2 + 1));
#define D(x, y) d[(x) + (y) * (len1 + 1)]
    for (size_t i = 0; i <= len1; ++i) {
        D(i, 0) = i;
    }
    for (size_t i = 1; i <= len2; ++i) {
        D(0, i) = i;
    }
    signed char *op = malloc(sizeof(signed char) * (len1 + 1) * (len2 + 1));
#define OP(x, y) op[(x) + (y) * (len1 + 1)]
    for (size_t i = 1; i <= len1; ++i) {
        OP(i, 0) = 0;
    }
    for (size_t i = 1; i <= len2; ++i) {
        OP(0, i) = 1;
    }
#else
    int *d = malloc(sizeof(int) * (len1 + 1) * 2);
#define D(x, y) d[(x) + ((y) & (size_t)0x1) * (len1 + 1)]
    for (size_t i = 0; i <= len1; ++i) {
        D(i, 0) = i;
    }
#endif

    for (size_t y = 1; y <= len2; ++y) {
#ifndef SHOW_STEP
        D(0, y) = y;
#endif
        for (size_t x = 1; x <= len1; ++x) {
            int cost = (int)(str1[x - 1] != str2[y - 1]);
            D(x, y) = min3(D(x - 1, y) + 1,
                           D(x, y - 1) + 1,
                           D(x - 1, y - 1) + cost);
#ifdef SHOW_STEP
            OP(x, y) = argmin3(D(x - 1, y) + 1,
                               D(x, y - 1) + 1,
                               D(x - 1, y - 1) + cost);
#endif
        }
    }

    printf("distance: %d\n", D(len1, len2));

#ifdef SHOW_STEP
    size_t x = len1;
    size_t y = len2;
    char **operations = malloc(sizeof(char *) * (len1 + len2));
    size_t op_i = 0;
    while (x != 0 || y != 0) {
        switch (OP(x, y)) {
        case 0: {
            char *desc = malloc(sizeof(char) * 9);
            sprintf(desc, "Delete %c", str1[x - 1]);
            operations[op_i++] = desc;
            --x;
            break;
        }
        case 1: {
            char *desc = malloc(sizeof(char) * 9);
            sprintf(desc, "Insert %c", str2[y - 1]);
            operations[op_i++] = desc;
            --y;
            break;
        }
        case 2: {
            if (str1[x - 1] == str2[y - 1]) {
                char *desc = malloc(sizeof(char) * 6);
                sprintf(desc, "Use %c", str1[x - 1]);
                operations[op_i++] = desc;
            } else {
                char *desc = malloc(sizeof(char) * 17);
                sprintf(desc, "Replace %c with %c", str1[x - 1], str2[y - 1]);
                operations[op_i++] = desc;
            }
            --x;
            --y;
            break;
        }
        }
    }

    for (size_t i = op_i; i > 0; --i) {
        puts(operations[i - 1]);
        free(operations[i - 1]);
    }

    free(operations);
    free(op);
#endif
    free(d);
}
