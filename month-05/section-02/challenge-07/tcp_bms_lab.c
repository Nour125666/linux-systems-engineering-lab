
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVICE_PORT 47631

static void fail(const char *what)
{
    perror(what);
    exit(EXIT_FAILURE);
}

static struct sockaddr_in endpoint(const char *ip)
{
    struct sockaddr_in address = {0};

    address.sin_family = AF_INET;
    address.sin_port = htons(SERVICE_PORT);

    if (inet_pton(AF_INET, ip,
                  &address.sin_addr) != 1) {
        fprintf(stderr, "Invalid IP: %s\n", ip);
        exit(EXIT_FAILURE);
    }

    return address;
}

static int run_server(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1)
        fail("server socket");

    int reuse = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   &reuse, sizeof(reuse)) == -1)
        fail("setsockopt");

    struct sockaddr_in local =
        endpoint("127.0.0.1");

    if (bind(fd, (struct sockaddr *)&local,
             sizeof(local)) == -1)
        fail("bind");

    if (listen(fd, 4) == -1)
        fail("listen");

    fprintf(stderr,
            "SERVER: waiting for client, PID=%ld\n",
            (long)getpid());

    int client_fd = accept(fd, NULL, NULL);

    if (client_fd == -1)
        fail("accept");

    const char response[] = "BMS READY\n";

    if (write(client_fd, response,
              sizeof(response) - 1) !=
        (ssize_t)(sizeof(response) - 1))
        fail("server write");

    if (close(client_fd) == -1)
        fail("close client");

    if (close(fd) == -1)
        fail("close server");

    return EXIT_SUCCESS;
}

static int run_client(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1)
        fail("client socket");

    struct sockaddr_in remote =
        endpoint("127.0.0.1");

    fprintf(stderr,
            "CLIENT: connecting to BMS gateway\n");

    if (connect(fd,
                (struct sockaddr *)&remote,
                sizeof(remote)) == -1) {
        perror("connect");
        (void)close(fd);
        return EXIT_FAILURE;
    }

    char buffer[64];

    ssize_t n = read(fd, buffer, sizeof(buffer));

    if (n == -1)
        fail("client read");

    if (n == 0) {
        fprintf(stderr,
                "CLIENT: connection closed without data\n");
        (void)close(fd);
        return EXIT_FAILURE;
    }

    printf("CLIENT: %.*s", (int)n, buffer);

    if (close(fd) == -1)
        fail("close client");

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s server|client\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "server") == 0)
        return run_server();

    if (strcmp(argv[1], "client") == 0)
        return run_client();

    fprintf(stderr, "Unknown mode\n");
    return EXIT_FAILURE;
}
