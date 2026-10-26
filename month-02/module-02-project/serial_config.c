#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int main(void)
{
    const char *device = "/tmp/ttyBMS1";

    int fd = open(device, O_RDWR | O_NOCTTY);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    struct termios tty;

    /* TODO 1: Read current terminal settings */
    if (tcgetattr( fd, &tty)==-1)
{ perror("tcgetattr");
close(fd);
return EXIT_FAILURE;}
    /* TODO 2: Configure raw mode */
 cfmakeraw(&tty);
    /* TODO 3: Set VMIN = 1 and VTIME = 0 */
tty.c_cc[VMIN]=1;
tty.c_cc[VTIME]=0;
    /* TODO 4: Apply the settings */
if (tcsetattr( fd, TCSANOW,
              &tty) ==-1){
perror("tcsetattr");
    close(fd);
    return EXIT_FAILURE;}
close(fd);
return EXIT_SUCCESS;}

