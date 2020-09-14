#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int gvar = 1; // global variable

int main(void) {
    pid_t pid;
    int svar = 1; // stack variable
    int *hvar = malloc(sizeof(int)); // heap variable
    *hvar = 30;
    pid = fork();
    if(pid < 0) {
        perror("fail to fork");
        exit(-1);
    }
    else if(pid == 0) { // subprocess
        gvar++;
        svar++;
        (*hvar)++;
        printf("subprocess, pid1: %u, pid2: %u, ppid: %u\n", pid, getpid(), getppid());
        printf("subprocess, global: %d, svar: %d, hvar: %d\n", gvar, svar, *hvar);
    }
    else { // parent process
        printf("parent, pid: %u, subprocess: %u\n", getpid(), pid);
        sleep(5);
        printf("parent, global: %d, svar: %d, hvar: %d\n", gvar, svar, *hvar);
    }
    return 0;
}