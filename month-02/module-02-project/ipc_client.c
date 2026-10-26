#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/bms.sock"

int main(void)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_un address = {0};
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_PATH);

    if (connect(fd,
                (struct sockaddr *)&address,
                sizeof(address)) == -1) {
        perror("connect");
        close(fd);
        return EXIT_FAILURE;
    }

    char buffer[128];

    ssize_t n = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (n == -1) {
        perror("recv");
        close(fd);
        return EXIT_FAILURE;
    }

    if (n == 0) {
        fprintf(stderr, "Server closed without sending data\n");
        close(fd);
        return EXIT_FAILURE;
    }

    buffer[n] = '\0';

    printf("Received: %s", buffer);

    close(fd);
    return EXIT_SUCCESS;
}
