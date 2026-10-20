
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char buffer[128];
    int fd = open(argv[1], O_RDONLY);
    ssize_t n;

    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, sizeof(buffer));
    }

    close(fd);
    return 0;
}
