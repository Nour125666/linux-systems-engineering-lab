
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

/* Read two bytes and display the current file offset. */
static int read_two(const char *label, int fd)
{
    char buffer[2];

    ssize_t n = read(fd, buffer, sizeof(buffer));

    if (n == -1) {
        perror("read");
        return -1;
    }

    off_t offset = lseek(fd, 0, SEEK_CUR);

    if (offset == (off_t)-1) {
        perror("lseek");
        return -1;
    }

    printf("%s: data='%.*s', offset=%jd\n",
           label, (int)n, buffer, (intmax_t)offset);

    return 0;
}

int main(void)
{
    int fd1 = -1, fd2 = -1, fd3 = -1;
    int status = 1;

    /* Open file for the first time */
    fd1 = open("data.bin", O_RDONLY);
    if (fd1 == -1) {
        perror("open fd1");
        goto cleanup;
    }

    /* Duplicate fd1: shared file offset */
    fd2 = dup(fd1);
    if (fd2 == -1) {
        perror("dup fd2");
        goto cleanup;
    }

    /* Open the same file again: independent offset */
    fd3 = open("data.bin", O_RDONLY);
    if (fd3 == -1) {
        perror("open fd3");
        goto cleanup;
    }

    /* Experiment 1: Three reads */
    if (read_two("fd1", fd1) == -1) goto cleanup;
    if (read_two("fd2", fd2) == -1) goto cleanup;
    if (read_two("fd3", fd3) == -1) goto cleanup;
if (lseek(fd1, 1, SEEK_SET) == (off_t)-1) {
    perror("lseek");
    goto cleanup;
}
    /* Experiment 2: Close fd1 */
    if (close(fd1) == -1) {
        perror("close fd1");
        fd1 = -1;
        goto cleanup;
    }
    fd1 = -1;
    /* fd2 is still valid, with the shared offset */
    if (read_two("fd2 after closing fd1", fd2) == -1)
        goto cleanup;

    status = 0;

cleanup:
    if (fd3 != -1 && close(fd3) == -1) {
        perror("close fd3");
        status = 1;
    }

    if (fd2 != -1 && close(fd2) == -1) {
        perror("close fd2");
        status = 1;
    }

    if (fd1 != -1 && close(fd1) == -1) {
        perror("close fd1");
        status = 1;
    }

    return status;
}
