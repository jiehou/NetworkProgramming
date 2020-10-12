#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]) {
    /*
    * fd[0]: read
    * fd[1]: write
    */
    int fd[2]; 

    if(pipe(fd) == -1) {
        perror("cannot create the pipe");
        exit(EXIT_FAILURE);
    }

    int id = fork();
    if(id == -1) {
        perror("process cannot be forked");
        exit(EXIT_FAILURE);
    }

    if(id == 0) { // child process
        close(fd[0]); // read pipe
        char* buffer = "hello world";
        printf("child process pid: %d, parent process pid: %d, sending: %s\n", getpid(), getppid(), buffer);
        write(fd[1], buffer, strlen(buffer)+1);
        close(fd[1]);
        exit(EXIT_SUCCESS);
    }
    else {
        close(fd[1]);
        char buffer[1024];
        if(read(fd[0], buffer, 1024) == -1) {
            perror("cannot read pipe");
            exit(EXIT_FAILURE);
        }
        close(fd[0]);
        printf("parent process pid: %d, received: %s\n", getpid(), buffer);
        exit(EXIT_SUCCESS);
    }
    return 0;
}