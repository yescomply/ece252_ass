#define _XOPEN_SOURCE 500 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <openssl/sha.h>
#include "dontmodify.h"

#define NUM_ELVES 5

/* Global Variables -- Add more if you need! */
int total_tasks;
int active_tasks;
int list_tasks;

pthread_t dobby_thread;
pthread_t elves[5];

pthread_mutex_t mutex;
sem_t empty_list;
sem_t full_list;
sem_t not_taking;
sem_t dobby_free;

int elf_ids[NUM_ELVES];
task* current_tasks[5];


/* Function Prototypes for pthreads */
void* dobby( void* );
void* house_elf( void * );

/* Don't change the function prototypes below;
   they are your API for how the house elves do work */

/* Removes a task from the list of things to do
   To be run by: house elf threads
   Takes no arguments
   NOT thread safe!
   return: a pointer to a task for an elf to do
*/
task* take_task();

/* Perform the provided task
   To be run by: house elf threads
   argument: pointer to task to be done
   IS thread safe
   return: nothing
*/  
void do_work( task* todo );

/* Put tasks in the list for elves to do
   To be run by: Dobby
   argument: how many tasks to put in the list
   NOT thread safe
   return: nothing
*/
void post_tasks( int howmany );

/* Used to unlock a mutex, if necessary, if a thread
   is cancelled when blocked on a semaphore
*/
void house_elf_cleanup( void * );



/* Complete the implementation of main() */


// int main( int argc, char** argv ) {

//   if ( argc != 2 ) {
//       printf( "Usage: %s total_tasks\n", argv[0] );
//       return -1;
//   }
//   /* Init global variables here */
//    total_tasks = atoi( argv[1] );
////////////////////TODO: remove on submission /////////////
int main() {
  /* Init global variables here */
   total_tasks = 20;
///////////////////////////////////////////////////////////

   active_tasks = total_tasks; // start with all tasks given
   list_tasks = 0;
   
   pthread_mutex_init(&mutex, NULL);
  
   sem_init(&empty_list, 0, 0);
   sem_init(&full_list, 0, 0);
   sem_init(&not_taking, 0, 1);
   sem_init(&dobby_free, 0, 1);

  
   printf("There are %d tasks to do today.\n", total_tasks);
  
  /* Launch threads here */

   pthread_create(&dobby_thread, NULL, dobby, NULL); //TODO: pass number of tasks properly

   for (int c = 0; c < 5; c++)
   {
      elf_ids[c] = c;
      pthread_create(&elves[c], NULL, house_elf, &elf_ids[c]); //using the number of a thread as its id
   }


  /* Wait for Dobby to be done */
  //Join all threads
   pthread_join(dobby_thread, NULL); //TODO: check





  /* Cleanup Global Variables here */
  
   pthread_mutex_destroy(&mutex);
   sem_destroy(&empty_list);
   sem_destroy(&full_list);
   sem_destroy(&not_taking);



  return 0;
}

/* Write the implementation of dobby() */

void* dobby( void * arg ) {
   // int tasks_topost = active_tasks - 10 > 0  ? 10 : active_tasks;
   // pthread_mutex_lock(&mutex);
   // list_tasks += tasks_topost;
   // active_tasks -= tasks_topost;
   // pthread_mutex_unlock(&mutex);

   while (1)
   {
      sem_wait(&empty_list); //wait for house elf to wake up Dobby until the list is empty
      // printf("Dobby saw empty list (%d) sem\n", list_tasks);
      if (active_tasks > 0)
      {

         
         
         
         int tasks_topost = active_tasks - 10 > 0  ? 10 : active_tasks;

         // printf("AS: Dobby woken up. Active tasks: %d \n", active_tasks);
         //When woken up, Dobby posts (up to) 10 tasks to do by calling post_tasks() 

         post_tasks(tasks_topost);
         // and updates the number of available tasks (active tasks). 
         pthread_mutex_lock(&mutex);
         list_tasks += tasks_topost;
         active_tasks -= tasks_topost;
         pthread_mutex_unlock(&mutex);

         
         // printf("Dobby thread just posted. Active tasks: %d \n", active_tasks);

         // After that, Dobby sleeps (is blocked).
         sem_post(&full_list); 
         for (int i = 0; i < NUM_ELVES; i++)
         {
            sem_post(&dobby_free);
         }
         
         // If there are fewer than 10 tasks remaining for the day then Dobby cannot post 10, but instead posts however many remain, and sleeps one last time.
      }

      if (active_tasks <= 0 && list_tasks <= 0) {
         // No more tasks for the day, cancel and join elves
         // printf("Dobby found no more tasks. Cancelling itself and elves. \n");
         for (int i = 0; i < NUM_ELVES; i++) {
               pthread_cancel(elves[i]);
         }
         for (int i = 0; i < NUM_ELVES; i++) {
               pthread_join(elves[i], NULL);
         }
         break; // exit the loop and end Dobby thread
      }
   }
}  

/* Complete the implementation of house_elf() */

void* house_elf( void * ignore ) {
  /* The following function call registers the cleanup
     handler to make sure that pthread_mutex_t locks 
     are unlocked if the thread is cancelled. It must
     be bracketed with a call to pthread_cleanup_pop 
     so don't change this line or the other one */
  pthread_cleanup_push( house_elf_cleanup, NULL ); 
   
   // sem_post(&empty_list);
   while( 1 ) 
   {
      int elf_num = *(int*) ignore;
      // sem_wait(&full_list); //wait for Dobby to post tasks
      

      if (list_tasks > 0)
      {        
         
         
         
         // //printf("AS: House elf %d waiting to take task. Active tasks: %d List tasks: %d \n", elf_num, active_tasks, list_tasks);

         sem_wait(&not_taking); //wait until no one is taking a task
         if (list_tasks > 0)
         {
            //printf("AS: House elf %d takes task \n", elf_num);
            current_tasks[elf_num] = take_task();
            pthread_mutex_lock(&mutex);
            // active_tasks -= 1;
            list_tasks -= 1;
            pthread_mutex_unlock(&mutex);
            //printf("AS: House elf %d took task. Starting work \n", elf_num);
            do_work(current_tasks[elf_num]); //can do in parallel
            //printf("AS: House elf %d just did a task. Active tasks: %d \n", elf_num, active_tasks);

         }
         else
         {
            //printf("AS: House elf %d tried taking task but all were done while waiting \n", elf_num);
         }
        
         sem_post(&not_taking); //took the task - others turns
         
         
      }
      else //list is empty
      {
         //printf("AS: House elf %d waiting for Dobby to free. No list tasks: %d \n", elf_num, list_tasks);
         sem_wait(&dobby_free);
         //printf("AS: House elf %d waited till Dobby is free. No list tasks: %d \n", elf_num, list_tasks);
         if (list_tasks <= 0)
         {
            //printf("AS: House elf %d Waking Dobby up now. No list tasks: %d \n", elf_num, list_tasks);

            //wake up dobby
            sem_post(&empty_list);
         }


      }
      
   }
  /* This cleans up the registration of the cleanup handler */
  pthread_cleanup_pop( 0 ) ;
}

/* Implement unlocking of any pthread_mutex_t mutex types
   that are locked in the house_elf thread, if any 
*/
void house_elf_cleanup( void* arg ) {


   pthread_mutex_unlock(&mutex);

}
