#include <stdio.h>
#include <pthread.h>  
#include <unistd.h> 
#include <stdlib.h>

void * thread_function(void * arg){
    long int r = *(int*)arg;
    int n = 0;
    printf("Start Thread %lu \n", r);
    while(n <10){
        sleep(1);
        n++;
        printf("inside Thread %lu %d\n", r, n);
    }
    printf("End Thread %lu\n", r);
    return (void *)r;
}

int main(){
    char line[100];
    int n_threads;
    printf("How many threads: ");
    fgets(line, 100, stdin);
    sscanf(line, "%d", &n_threads);
    pthread_t thread_id;

    pthread_t threads[100];
    void* thread_return;
    int return_array[100];

    int i = 0;
    while( i < n_threads) {
        pthread_create(&thread_id, NULL, thread_function, &i);
        threads[i] = thread_id;
        i++;
    };

    for(i=0; i < n_threads; i++){

        pthread_join(threads[i], &thread_return);
        return_array[i] = (int) thread_return;
        printf("thread %d: %d.\n", i, return_array[i]);
    }

    printf("Carregue em enter para terminar\n");
    fgets(line, 100, stdin);
    exit(0);
}