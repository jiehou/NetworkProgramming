#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    pid_t pid;
    pid = fork();
    if(pid < 0) {
        perror("fail to fork");
        exit(-1);
    }
    else if (pid == 0) {
        printf("subprocess, pid: %u, ppid: %u\n", getpid(), getppid());
    }
    else {
        printf("parent, pid: %u, subprocess pid: %u\n", getpid(), pid);
        sleep(5);
    }
    return 0;
}