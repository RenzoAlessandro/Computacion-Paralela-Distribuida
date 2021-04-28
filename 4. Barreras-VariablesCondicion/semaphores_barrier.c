/* Archivo:  
 *    semaphores_barrier.c
 *
 * Propósito:
 *    Utilizar barreras de semaphore para sincronizar threads.
 *
 * Entrada:
 *    Ninguna
 * Salida:
 *    Time for BARRIER_COUNT barriers
 *
 * Compilar:
 *    gcc -g -Wall -o ejecutable semaphores_barrier.c -lpthread
 *    timer.h needs to be available
 *
 * Ejecutar:
 *    ./ejecutable <thread_count>
 *
 * Nota:
 *    La configuración del indicador del compilador -DEBUG hará que se
 *    imprima un mensaje después de completar cada barrera.
 *
 * IPP:   Sección 4.8.2  (págs. 177 y sigs.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "timer.h"

#define BARRIER_COUNT 100

int thread_count;
int counter;
sem_t barrier_sems[BARRIER_COUNT];
sem_t count_sem;

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
      sem_init(&barrier_sems[i], 0, 0);
   sem_init(&count_sem, 0, 1);

   GET_TIME(start);
   for (thread = 0; thread < thread_count; thread++)
      pthread_create(&thread_handles[thread], (pthread_attr_t*) NULL,
          Thread_work, (void*) thread);

   for (thread = 0; thread < thread_count; thread++) {
      pthread_join(thread_handles[thread], NULL);
   }
   GET_TIME(finish);
   printf("Tiempo transcurrido = %e segundos\n", finish - start);

   sem_destroy(&count_sem);
   for (i = 0; i < BARRIER_COUNT; i++)
      sem_destroy(&barrier_sems[i]);
   free(thread_handles);
   return 0;
}  /* main */


/*--------------------------------------------------------------------
 * Función:    Usage
 * Propósito:  Imprimir línea de comando para función y terminar
 * En arg:     prog_name
 */
void Usage(char* prog_name) {

   fprintf(stderr, "Usar: %s <numero de threads>\n", prog_name);
   exit(0);
}  /* Usage */


/*-------------------------------------------------------------------
 * Función:     Thread_work
 * Propósito:   Ejecutar las barreras BARRIER_COUNT
 * En arg:      rank
 * Var Global:  thread_count, count, barrier_sems, count_sem
 * Var Retorno: Ignorado
 */
void *Thread_work(void* rank) {
#  ifdef DEBUG
   long my_rank = (long) rank;
#  endif
   int i, j;

   for (i = 0; i < BARRIER_COUNT; i++) {
      // Barrera
      sem_wait(&count_sem);
      if (counter == thread_count - 1) {
         counter = 0;
         sem_post(&count_sem);
         for (j = 0; j < thread_count-1; j++)
            sem_post(&barrier_sems[i]);
      } else {
         counter++;
         sem_post(&count_sem);
         sem_wait(&barrier_sems[i]);
      }
#     ifdef DEBUG
      if (my_rank == 0) {
         printf("Todos los threads completaron la barrera %d\n", i);
         fflush(stdout);
      }
#     endif
   }

   return NULL;
}  /* Thread_work */
