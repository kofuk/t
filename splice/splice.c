#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in sockaddr = {0};
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(8000); // :8000
    struct in_addr addr = {0}; // 0.0.0.0
    sockaddr.sin_addr = addr;

    if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)) < 0) {
        perror("bin");
        return 1;
    }

    if (listen(sock, 0)) {
        perror("listen");
        return 1;
    }

    for (;;) {
        fputs("waiting for a new connection...\n", stderr);

        struct sockaddr_in peer;
        socklen_t peer_addr_len = sizeof(struct sockaddr_in);
        int conn = accept(sock, (struct sockaddr *)&peer, &peer_addr_len);

        size_t bytes_xferred = 0;
        struct timeval start;
        gettimeofday(&start, NULL);

        for (;;) {
#if 0
            char buf[4096];
            ssize_t n_read = read(conn, buf, 4096);
            if (n_read <= 0) {
                break;
            }
            bytes_xferred += (size_t)n_read;

            write(STDOUT_FILENO, buf, (size_t)n_read);
#else
            // splice(2) requires either source or drain to be a pipe.
            // To try splice implementation, run this program as follows (or redirect to named pipes):
            // $ ./splice | cat
            ssize_t n_spliced = splice(conn, NULL, STDOUT_FILENO, NULL, 4096, 0);
            if (n_spliced < 0) {
                perror("splice");
                break;
            } else if (n_spliced == 0) {
                break;
            }
            bytes_xferred += (size_t)n_spliced;
#endif
        }

        struct timeval end;
        gettimeofday(&end, NULL);

        close(conn);

        time_t s_diff = end.tv_sec - start.tv_sec;
        int us_diff = (int)(end.tv_usec - start.tv_usec);

        double sec = s_diff + us_diff / 1000.0 / 1000.0;

        bytes_xferred *= 8;

        fprintf(stderr, "%lf bps\n", bytes_xferred / sec);
    }
}
