#include <pthread.h>
#include <stdio.h>

int gshared = 0;

void* thread_run(void* arg) {
    int* var = (int*) arg;
    printf("[I] tid: %d\n", pthread_self());
    for(int i = 0; i < 100; ++i) {
        *var += 1;
        gshared += 1;
    }
}

int main(void) {
    int lshared = 0;
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, thread_run, &lshared);
    pthread_create(&tid2, NULL, thread_run, &lshared);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    printf("[I] lshared: %d\n", lshared);
    printf("[I] gshared: %d\n", gshared);
    return 0;
}