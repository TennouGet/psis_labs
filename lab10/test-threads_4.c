#include <stdio.h>
#include <pthread.h>  
#include <unistd.h> 
#include <stdlib.h>
int n = 0;

void * thread_function(void * arg){
    int time = *(int*) arg;
    long int n2=0;
    long int r = random()%100;
    printf("Start Thread %lu\n", r);
    while(n2 <10){
        sleep(time);
        n2++;
        printf("inside Thread %lu %d %d\n", r, n, time);
    }
    printf("End Thread %lu\n", r);
    return (void *)n2;
}

int main(){
    char line[100];
    int n_threads;
    printf("How many threads: ");
    fgets(line, 100, stdin);
    sscanf(line, "%d", &n_threads);
    pthread_t thread_id;

    int *time = malloc(sizeof(int));
    pthread_t threads[100];
    void* thread_return;
    long int return_array[100];

    int i = 0;
    while( i < n_threads) {
        *time = random() % 5;
        pthread_create(&thread_id, NULL,thread_function, time);
        i++;
    };

    for(i=0; i < n_threads; i++){

        pthread_join(threads[i], &thread_return);
        return_array[i] = (long int) thread_return;
        printf("thread %d: %ld.\n", i, return_array[i]);
        n = n + return_array[i];
    }

    printf("n: %d.\n", n);

    printf("Carregue em enter para terminar\n");
    fgets(line, 100, stdin);
    exit(0);
}