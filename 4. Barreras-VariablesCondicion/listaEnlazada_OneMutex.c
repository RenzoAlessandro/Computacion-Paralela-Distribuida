/* Archivo:   listaEnlazada_OneMutex.c
 *
 * Propósito: Implementar una lista enlazada ordenada de múltiples subprocesos de entradas 
 *            con ops insertar, imprimir, miembro, eliminar, lista libre. 
 *            Esta versión usa un solo mutex
 * 
 * Compilar:  gcc -g -Wall -o ejecutable listaEnlazada_OneMutex.c my_rand.c -lpthread
 *            se necesita timer.h y my_rand.h
 *
 * Ejecutar:  ./ejecutable <thread_count>
 * Entrada:   Número total de keys insertadas por thread principal
 *            número total de operaciones realizadas por cada hilo 
 *            (todos los hilos llevan a cabo el mismo número de operaciones)
 *            porcentaje de operaciones que son búsquedas e inserciones 
 *            (las operaciones restantes son eliminaciones).
 * Salida:    Tiempo transcurrido para realizar las operaciones
 *
 * Notas:
 *    1. No se permiten valores repetidos en la lista.
 *    2. Indicador de compilación DEBUG utilizado. Para obtener la salida de 
 *       depuración, compile con el indicador de línea de comando -DDEBUG.
 *    3. Utiliza un mutex para controlar el acceso a la lista.
 *    4. La función aleatoria no es segura para subprocesos. Entonces, 
 *       este programa usa un generador congruencial lineal simple.
 *    5. La bandera -DOUTPUT  a gcc mostrará la lista antes y después de que 
 *       los subprocesos hayan trabajado en ella.
 *
 * IPP:   Sección 4.9.2 (pp. 185 and ff.)
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "my_rand.h"
#include "timer.h"

/* Los ints aleatorias son inferiores a MAX_KEY */
const int MAX_KEY = 100000000;

/* Estructura para nodos de la lista */
struct list_node_s {
   int    data;
   struct list_node_s* next;
};

/* Variables compartidas */
struct      list_node_s* head = NULL;  
int         thread_count;
int         total_ops;
double      insert_percent;
double      search_percent;
double      delete_percent;
pthread_mutex_t mutex;
pthread_mutex_t count_mutex;
int         member_total=0, insert_total=0, delete_total=0;

/* Setup y cleanup */
void        Usage(char* prog_name);
void        Get_input(int* inserts_in_main_p);

/* Función de Thread */
void*       Thread_work(void* rank);

/* Lista de operaciones */
int         Insert(int value);
void        Print(void);
int         Member(int value);
int         Delete(int value);
void        Free_list(void);
int         Is_empty(void);

/*-----------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   long i; 
   int key, success, attempts;
   pthread_t* thread_handles;
   int inserts_in_main;
   unsigned seed = 1;
   double start, finish;

   if (argc != 2) Usage(argv[0]);
   thread_count = strtol(argv[1],NULL,10);

   Get_input(&inserts_in_main);

   /* Intenta insertar keys inserts_in_main, pero abandona despues */
   /* 2*inserts_in_main intentos.                                  */
   i = attempts = 0;
   while ( i < inserts_in_main && attempts < 2*inserts_in_main ) {
      key = my_rand(&seed) % MAX_KEY;
      success = Insert(key);
      attempts++;
      if (success) i++;
   }
   printf("Keys %ld insertadas en la lista vacia\n", i);

#  ifdef OUTPUT
   printf("Antes de comenzar los threads, lista = \n");
   Print();
   printf("\n");
#  endif

   thread_handles = malloc(thread_count*sizeof(pthread_t));
   pthread_mutex_init(&mutex, NULL);
   pthread_mutex_init(&count_mutex, NULL);

   GET_TIME(start);
   for (i = 0; i < thread_count; i++)
      pthread_create(&thread_handles[i], NULL, Thread_work, (void*) i);

   for (i = 0; i < thread_count; i++)
      pthread_join(thread_handles[i], NULL);
   GET_TIME(finish);
   printf("Tiempo transcurrido = %e seconds\n", finish - start);
   printf("Total de operaciones = %d\n", total_ops);
   printf("Operaciones Miembro  = %d\n", member_total);
   printf("Operaciones Insertar = %d\n", insert_total);
   printf("Operaciones Eliminar = %d\n", delete_total);

#  ifdef OUTPUT
   printf("Después de que terminan los threads, lista = \n");
   Print();
   printf("\n");
#  endif

   Free_list();
   pthread_mutex_destroy(&mutex);
   pthread_mutex_destroy(&count_mutex);
   free(thread_handles);

   return 0;
}  /* main */


/*-----------------------------------------------------------------*/
void Usage(char* prog_name) {
   fprintf(stderr, "Usar: %s <thread_count>\n", prog_name);
   exit(0);
}  /* Usar */

/*-----------------------------------------------------------------*/
void Get_input(int* inserts_in_main_p) {

   printf("¿Cuántas keys se deben insertar en el thread principal?\n");
   scanf("%d", inserts_in_main_p);
   printf("¿Cuántas operaciones en total se deben ejecutar?\n");
   scanf("%d", &total_ops);
   printf("¿Porcentaje de operaciones que deberían ser búsquedas? (entre 0 y 1)\n");
   scanf("%lf", &search_percent);
   printf("¿Porcentaje de operaciones que deberían ser inserciones? (entre 0 y 1)\n");
   scanf("%lf", &insert_percent);
   delete_percent = 1.0 - (search_percent + insert_percent);
}  /* Get_input */

/*-----------------------------------------------------------------*/
/* Inserta el valor en la ubicación numérica correcta en la lista */
/* Si el valor no está en la lista, devuelve 1, de lo contrario, devuelve 0 */
int Insert(int value) {
   struct list_node_s* curr = head;
   struct list_node_s* pred = NULL;
   struct list_node_s* temp;
   int rv = 1;
   
   while (curr != NULL && curr->data < value) {
      pred = curr;
      curr = curr->next;
   }

   if (curr == NULL || curr->data > value) {
      temp = malloc(sizeof(struct list_node_s));
      temp->data = value;
      temp->next = curr;
      if (pred == NULL)
         head = temp;
      else
         pred->next = temp;
   } else { /* valor en la lista */
      rv = 0;
   }

   return rv;
}  /* Insertar */

/*-----------------------------------------------------------------*/
void Print(void) {
   struct list_node_s* temp;

   printf("list = ");

   temp = head;
   while (temp != (struct list_node_s*) NULL) {
      printf("%d ", temp->data);
      temp = temp->next;
   }
   printf("\n");
}  /* Imprimir */


/*-----------------------------------------------------------------*/
int  Member(int value) {
   struct list_node_s* temp;

   temp = head;
   while (temp != NULL && temp->data < value)
      temp = temp->next;

   if (temp == NULL || temp->data > value) {
#     ifdef DEBUG
      printf("%d no esta en la lista\n", value);
#     endif
      return 0;
   } else {
#     ifdef DEBUG
      printf("%d esta en la lista\n", value);
#     endif
      return 1;
   }
}  /* Miembro */

/*-----------------------------------------------------------------*/
/* Elimina valor de la lista */
/* Si el valor está en la lista, devuelve 1, de lo contrario, devuelve 0 */
int Delete(int value) {
   struct list_node_s* curr = head;
   struct list_node_s* pred = NULL;
   int rv = 1;

   /* Encuentra valor */
   while (curr != NULL && curr->data < value) {
      pred = curr;
      curr = curr->next;
   }
   
   if (curr != NULL && curr->data == value) {
      if (pred == NULL) { /* primer elemento de la lista */
         head = curr->next;
#        ifdef DEBUG
         printf("Liberado %d\n", value);
#        endif
         free(curr);
      } else { 
         pred->next = curr->next;
#        ifdef DEBUG
         printf("Liberado %d\n", value);
#        endif
         free(curr);
      }
   } else { /* No en lista */
      rv = 0;
   }

   return rv;
}  /* Eliminar */

/*-----------------------------------------------------------------*/
void Free_list(void) {
   struct list_node_s* current;
   struct list_node_s* following;

   if (Is_empty()) return;
   current = head; 
   following = current->next;
   while (following != NULL) {
#     ifdef DEBUG
      printf("Liberado %d\n", current->data);
#     endif
      free(current);
      current = following;
      following = current->next;
   }
#  ifdef DEBUG
   printf("Liberado %d\n", current->data);
#  endif
   free(current);
}  /* Free_list */

/*-----------------------------------------------------------------*/
int  Is_empty(void) {
   if (head == NULL)
      return 1;
   else
      return 0;
}  /* Es vacio */

/*-----------------------------------------------------------------*/
void* Thread_work(void* rank) {
   long my_rank = (long) rank;
   int i, val;
   double which_op;
   unsigned seed = my_rank + 1;
   int my_member=0, my_insert=0, my_delete=0;
   int ops_per_thread = total_ops/thread_count;

   for (i = 0; i < ops_per_thread; i++) {
      which_op = my_drand(&seed);
      val = my_rand(&seed) % MAX_KEY;
      if (which_op < search_percent) {
         pthread_mutex_lock(&mutex);
         Member(val);
         pthread_mutex_unlock(&mutex);
         my_member++;
      } else if (which_op < search_percent + insert_percent) {
         pthread_mutex_lock(&mutex);
         Insert(val);
         pthread_mutex_unlock(&mutex);
         my_insert++;
      } else { /* delete */
         pthread_mutex_lock(&mutex);
         Delete(val);
         pthread_mutex_unlock(&mutex);
         my_delete++;
      }
   }  /* for */

   pthread_mutex_lock(&count_mutex);
   member_total += my_member;
   insert_total += my_insert;
   delete_total += my_delete;
   pthread_mutex_unlock(&count_mutex);

   return NULL;
}  /* Thread_work */
