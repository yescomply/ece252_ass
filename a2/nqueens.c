// from the Cilk manual: http://supertech.csail.mit.edu/cilk/manual-5.4.6.pdf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int safe(char * config, int i, int j)
{
    int r, s;

    for (r = 0; r < i; r++)
    {
        s = config[r];
        if (j == s || i-r==j-s || i-r==s-j)
            return 0;
    }
    return 1;
}

int count = 0;
/*Multithreading START*/

pthread_mutex_t mutex;
//TODO: create datastructure to contain the function args for the threads


/*Multithreading END*/

void nqueens(char *config, int n, int i)
{
    char *new_config;
    int j;

    if (i==n)
    {
        /*Multithreading START*/
        count++; //TODO: lock with mutex
        /*Multithreading END*/
    }
    
    /* try each possible position for queen <i> */
    for (j=0; j<n; j++) //columns
    {
        /* allocate a temporary array and copy the config into it */
        new_config = malloc((i+1)*sizeof(char));
        memcpy(new_config, config, i*sizeof(char)); //TODO: check if need to lock this memory - if configs must be shared across threads at all
        if (safe(new_config, i, j))
        {
            new_config[i] = j;
	        nqueens(new_config, n, i+1);
        }
        free(new_config);
    }
    return;
}

int main(int argc, char *argv[])
{
    int n;
    char *config;

    if (argc < 2)
    {
        printf("%s: number of queens required\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);
    config = malloc(n * sizeof(char));

    printf("running queens %d\n", n);
    /*Multithreading (mine) START*/

    //TODO: create the N threads with N different configs (for the first row, i.e. i=0)
    //TODO: call the nqueens on each thread starting at i=1
    //pthread_create( pthread_t *thread, const pthread_attr_t * attr, void *(*start_routine)( void * ), void *arg );

    nqueens(config, n, 0);
    /*Multithrading END*/
    printf("# solutions: %d\n", count);

    return 0;
}
