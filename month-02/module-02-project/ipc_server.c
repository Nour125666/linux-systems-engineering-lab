#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/bms.sock"

int main(void)
{
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_un address = {0};

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_PATH);

    unlink(SOCKET_PATH);

    if (bind(server_fd,
             (struct sockaddr *)&address,
             sizeof(address)) == -1) {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        return EXIT_FAILURE;
    }

    printf("BMS server listening on %s\n", SOCKET_PATH);
    fflush(stdout);

    int client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        unlink(SOCKET_PATH);
        return EXIT_FAILURE;
    }

    const char message[] = "BMS voltage: 51.200 V\n";

    ssize_t sent = send(
        client_fd,
        message,
        sizeof(message) - 1,
        0
    );

    if (sent == -1) {
        perror("send");
    } else {
        printf("Sent %zd bytes to client\n", sent);
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    return EXIT_SUCCESS;
}
