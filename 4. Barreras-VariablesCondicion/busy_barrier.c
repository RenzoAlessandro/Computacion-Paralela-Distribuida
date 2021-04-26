/* Archivo:  
 *    busy_barrier.c 
 *
 * Propósito:
 *    Utilizar barreras busy ocupada para sincronizar threads.
 *
 * Input:
 *    ninguno
 * Output:
 *    Time for BARRIER_COUNT barriers
 *
 * Compile:
 *    gcc -g -Wall -o ejecutable busy_barrier.c -lpthread
 * Usage:
 *    ./ejecutable <thread_count>
 *
 * Nota:
 *    El flag compilacion DEBUG imprimirá un mensaje después de cada barrera    
 *
 * IPP:   Sección 4.8.1 (págs. 177)
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

#define BARRIER_COUNT 100

int thread_count;
int barrier_thread_counts[BARRIER_COUNT];
pthread_mutex_t barrier_mutex;

void Usage(char* prog_name);
void *Thread_work(void* rank);

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   long       thread, i;
   pthread_t* thread_handles; 
   double start, finish;

   if (argc != 2)
      Usage(argv[0]);
   thread_count = strtol(argv[1], NULL, 10);

   thread_handles = malloc (thread_count*sizeof(pthread_t));
   for (i = 0; i < BARRIER_COUNT; i++)
      barrier_thread_counts[i] = 0;
   pthread_mutex_init(&barrier_mutex, NULL);

   GET_TIME(start);
   for (thread = 0; thread < thread_count; thread++)
      pthread_create(&thread_handles[thread], NULL,
          Thread_work, (void*) thread);

   for (thread = 0; thread < thread_count; thread++) {
      pthread_join(thread_handles[thread], NULL);
   }
   GET_TIME(finish);
   printf("Tiempo transcurrido = %e segundos\n", finish - start);

   pthread_mutex_destroy(&barrier_mutex);
   free(thread_handles);
   return 0;
}  /* main */


/*--------------------------------------------------------------------
 * Función:     Usage
 * Propósito:   Imprimir línea de comando para función y terminar
 * En arg:      prog_name
 */
void Usage(char* prog_name) {

   fprintf(stderr, "Usar: %s <numero de threads>\n", prog_name);
   exit(0);
}  /* Usage */


/*-------------------------------------------------------------------
 * Función:     Thread_work
 * Propósito:   Ejecutar las barreras BARRIER_COUNT
 * En arg:      rank
 * Var Global:  thread_count, barrier_thread_counts, barrier_mutex
 * Val Retorno: Ignorado
 */
void *Thread_work(void* rank) {
#  ifdef DEBUG
   long my_rank = (long) rank; 
#  endif
   int i;

   for (i = 0; i < BARRIER_COUNT; i++) {
      pthread_mutex_lock(&barrier_mutex);
      barrier_thread_counts[i]++;
      pthread_mutex_unlock(&barrier_mutex);
      while (barrier_thread_counts[i] < thread_count);
#     ifdef DEBUG
      if (my_rank == 0) {
         printf("Todos los threads entraron en la barrera %d\n", i);
         fflush(stdout);
      }
#     endif
   }

   return NULL;
}  /* Thread_work */
