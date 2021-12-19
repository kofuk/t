#include <stdio.h>
#include <string.h>

#include <sys/mman.h>
#include <time.h>

int main(void) {
    size_t total = 0;

    struct timespec rqtp = {0};
    rqtp.tv_sec = 0;
    rqtp.tv_nsec = 1000 * 1000;

    for (;;) {
        void *addr = mmap(NULL, 512 * 1024,
                          PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
        total += 512;
        if (addr == MAP_FAILED) {
            perror("\nmmap");
            break;
        }

#pragma omp parallel for
        for (size_t i = 0; i < 512; ++i) {
            memset(addr + i * 1024, 0, 1024);
        }

        printf("%ld MiB\n", total / 1024);
        fflush(stdout);

        nanosleep(&rqtp, NULL);
    }
}
