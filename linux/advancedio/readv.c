#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>

int main() {
    char foo[48], bar[100];
    struct iovec iov[2];
    ssize_t nr;
    int fd;
    fd = open("buccaneer.txt", O_RDONLY);
    if(fd == -1) {
        perror("open");
        return 1;
    }
    /* set up iovec structures */
    iov[0].iov_base = foo;
    iov[0].iov_len = sizeof(foo);
    iov[1].iov_base = bar;
    iov[1].iov_len = sizeof(bar);
    /* read into the structures with a single call */
    nr = readv(fd, iov, 2);
    if(nr == -1) {
        perror("readv");
        return 1;
    }
    for(int i = 0; i < 2; ++i) {
        printf("%d, %s", i, (char*)iov[i].iov_base);
    }
    if(close(fd)) {
        perror("close");
        return 1;
    }
    return 0;
}