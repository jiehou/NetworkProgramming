/**
 * a signal is a notification sent to a process to notify it that an event occured
 * e.g. ctrl+c
*/
/**
 * return: it is a language keyword. return from the current function
 * exit: terminate the whole program. in other words, terminate the current process.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handler(int num) {
    write(STDOUT_FILENO, "i will not die\n", 13);
    exit(1);
}

int main() {
    signal(SIGINT, handler);
    while(1) {
        printf("wasting your cycles: %d\n", getpid());
        sleep(5);
    }
    return 0;
}