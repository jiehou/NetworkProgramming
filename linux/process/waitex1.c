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
        printf("pp: pid %d\n", getpid());
        int status = -1;
        pid_t pr = wait(&status);
        printf("pp: catched a child process with pid %d\n", pr);
        if(WIFEXITED(status))
            printf("pp: child process %d exit normally with return code %d\n", pr, WEXITSTATUS(status));
        else 
            printf("pp: child process %d exit abnormally\n", pr);
    }
    else {
        printf("cp: pid %d, ppid %d\n", getpid(), getppid());
        sleep(10);
        exit(5);
    }
}