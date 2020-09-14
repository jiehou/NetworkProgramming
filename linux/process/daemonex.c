#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    if(pid < 0) {
        perror("fail to fork");
        exit(-1);
    }
    else if(pid > 0) { // parent process
        printf("parent, sleep for sometime\n");
        sleep(5);
        printf("parent, id: %u, pid: %u", getpid(), pid);
    }
    else { // child process
        printf("child process exit\n");
        exit(0);
    }
    return 0;
}