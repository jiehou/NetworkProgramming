#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

const int kTimeout = 5; /* poll timeout (in seconds) */

int main(void) {
    struct pollfd fds[2];
    int ret;
    /* watch stdin for input */
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    /* watch stdout for ability to wirte */
    fds[1].fd = STDOUT_FILENO;
    fds[1].events = POLLOUT;
    ret = poll(fds, 2, kTimeout * 1000);
    if(ret == -1) error(1, errno, "poll failed");
    if(ret == 0) error(1, 0, "timeout\n");
    if(fds[0].revents & POLLIN) printf("stdin is readable\n");
    if(fds[1].revents & POLLOUT) printf("stdout is writeable\n");
    return 0;
}