#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"
 
#define NUM_THREADS  2

typedef struct filaint_t {
    struct filaint_t *prev;
    struct filaint_t *next;
    int value;
} filaint_t;

// Alternância de uso
int turn = 0;
 
int sum = 0 ;
filaint_t vector[10];
filaint_t *queue = NULL;

void print_elem (void *ptr) {
    filaint_t *elem = ptr ;

    if (!elem)
        return ;

    elem->prev ? printf ("%d", elem->prev->value) : printf ("*") ;
    printf ("<%d>", elem->value) ;
    elem->next ? printf ("%d", elem->next->value) : printf ("*") ;
}

void enter(int task_id) {
    while (turn != task_id) {
        
    }
}

void leave(int task_id) {
    turn = (turn + 1) % NUM_THREADS;
}
 
void threadBody (void *id)
{

    filaint_t* old = NULL;
    filaint_t new;
    while (1) {
        enter((int)id);
        new.prev = NULL;
        new.next = NULL;
        new.value = rand() % 100;
        old = queue;
        printf("Before remove %d\n", queue->value);
        //queue_print("", (queue_t*)queue, print_elem);
        queue_remove((queue_t**)&queue, (queue_t*)old);
        printf("Ater remove %d\n", queue->value);
        queue_append((queue_t**)queue, (queue_t*)&new);
        printf("thread %d: tira %d, põe %d, fila: ", id, old->value, new.value);
        queue_print("", (queue_t*)queue, print_elem);
        leave((int)id);
    }
 
   pthread_exit (NULL) ;

}

int main (int argc, char *argv[])
{
   pthread_t thread [NUM_THREADS] ;
   pthread_attr_t attr ;
   long i, status ;

   srand(time(NULL));

   for (int j = 0; j < 10; j++) {
       vector[j].prev = NULL;
       vector[j].next = NULL;
       vector[j].value = rand() % 100;
       queue_append((queue_t**)&queue, (queue_t*)&vector[j]);
   }
 
   pthread_attr_init (&attr) ;
   pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE) ;
 
   // create threads
   for(i=0; i<NUM_THREADS; i++)
   {
      status = pthread_create (&thread[i], &attr, (void *) threadBody, (void *) i) ;
      if (status)
      {
         perror ("pthread_create") ;
         exit (1) ;
      }
   }
 
   // wait all threads to finish   
   for (i=0; i<NUM_THREADS; i++)
   {
      status = pthread_join (thread[i], NULL) ;
      if (status)
      {
         perror ("pthread_join") ;
         exit (1) ;
      }
   }
 
   printf ("Sum should be %d and is %d\n", NUM_THREADS*NUM_STEPS, sum) ;
 
   pthread_attr_destroy (&attr) ;
   pthread_exit (NULL) ;
}