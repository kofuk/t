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

int main(int argc, char **argv) {
    if (argc < 3) {
        puts("usage: levenshtein_distance <str1> <str2>");
        return 1;
    }

    char *str1 = argv[1];
    char *str2 = argv[2];
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    int *d = malloc(sizeof(int) * (len1 + 1) * (len2 + 1));
    for (size_t i = 0; i <= len1; ++i) {
        d[i] = i;
    }
    for (size_t i = 1; i <= len2; ++i) {
        d[i * len1] = i;
    }

    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            int cost = (int)(str1[i - 1] != str2[i - 1]);
            d[j * len1 + i] = min3(d[j * len1 + i - 1] + 1,
                                   d[(j - 1) * len1 + i] + 1,
                                   d[(j - 1) * len1 + i - 1] + cost);
        }
    }
    printf("distance: %d\n", d[len1 * len2 + len1]);
}
