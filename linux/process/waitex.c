#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();
    if(pid < 0) {
        perror("fork error\n");
        return 0;
    }
    else if(pid > 0) { // parent process
        printf("parent process: pid %d\n", getpid());
        pid_t pr = wait(NULL);
        printf("parent process: catched a child process with pid %d\n", pr);
    }
    else {
        printf("child process: pid %d, ppid %d\n", getpid(), getppid());
        exit(0);
    }
}