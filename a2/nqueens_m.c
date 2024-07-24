#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int safe(char * config, int i, int j) {
    int r, s;
    for (r = 0; r < i; r++) {
        s = config[r];
        if (j == s || i-r==j-s || i-r==s-j)
            return 0;
    }
    return 1;
}

int count = 0;
pthread_mutex_t mutex;

typedef struct {
    char* conf;
    int n;
    int i;
} ThreadParams;

void* nqueens(void* arg) {
    ThreadParams* args = (ThreadParams*)arg;
    char* config = args->conf;
    int n = args->n;
    int i = args->i;

    char *new_config;
    int j;

    if (i == n) {
        pthread_mutex_lock(&mutex);
        count++;
        pthread_mutex_unlock(&mutex);
    }

    for (j = 0; j < n; j++) {
        new_config = malloc((i+1) * sizeof(char));
        memcpy(new_config, config, i * sizeof(char));
        if (safe(new_config, i, j)) {
            new_config[i] = j;

            ThreadParams new_args;
            new_args.conf = new_config;
            new_args.i = i + 1;
            new_args.n = n;
            nqueens(&new_args);
        }
        free(new_config);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int n;
    char *config;

    if (argc < 2) {
        printf("%s: number of queens required\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);
    config = malloc(n * sizeof(char));

    printf("running queens %d\n", n);

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[n];
    ThreadParams thread_args[n];

    for (int c = 0; c < n; c++) {
        thread_args[c].conf = malloc(n * sizeof(char)); // Allocate memory for this thread's config
        thread_args[c].conf[0] = c; //Fix first queen
        thread_args[c].n = n;
        thread_args[c].i = 1; //start from the second row, i=1

        pthread_create(&threads[c], NULL, nqueens, &thread_args[c]);
    }

    //Join all threads
    for (int c = 0; c < n; c++) {
        pthread_join(threads[c], NULL);
    }

    pthread_mutex_destroy(&mutex);

    printf("Found # solutions: %d\n", count);

    //free all configs memory
    for (int c = 0; c < n; c++) {
        free(thread_args[c].conf);
    }
    free(config);

    return 0;
}
