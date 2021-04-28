/* Archivo:   listaEnlazada_MultiMutex.c
 *
 * Propósito: Implemente una lista enlazada ordenada de múltiples subprocesos de 
 *            entradas con ops insertar, imprimir, miembro, eliminar, lista libre. 
 *            Esta versión usa un mutex por nodo de lista
 * 
 * Compilar:  gcc -g -Wall -I. -o ejecutable listaEnlazada_MultiMutex.c my_rand.c -lpthread
 *            se necesita timer.h y my_rand.h
 *
 * Ejecutar:  ./ejecutable <thread_count>
 * Entrada:   Número total de keys insertadas por thread principal
 *            Número total de operaciones realizadas por cada hilo 
 *            (todos los hilos llevan a cabo el mismo número de operaciones)
 *            porcentaje de operaciones que son búsquedas e inserciones 
 *            (las operaciones restantes son eliminaciones).
 * Salida:    Tiempo transcurrido para realizar las operaciones
 *
 * Notas:
 *    1. No se permiten valores repetidos en la lista.
 *    2. Indicador de compilación DEBUG utilizado. Para obtener la salida de 
 *       depuración, compile con el indicador de línea de comando -DDEBUG.
 *    3. Utiliza un mutex por nodo para controlar el acceso a la lista
 *    4. La función aleatoria no es segura para subprocesos. Entonces, 
 *       este programa usa un generador congruencial lineal simple.
 *    5. La bandera -DOUTPUT  a gcc mostrará la lista antes y después de que 
 *       los subprocesos hayan trabajado en ella.
 *    6. Solo Insert, Member y Delete usan bloqueos: Print y Free_List *no* 
 *       se deben llamar cuando varios subprocesos están accediendo a la lista.
 *    7. Steffen Christgau y Bettina Schnor señalaron algunos errores en las 
 *       implementaciones de los recorridos de la lista. Estos se corrigieron 
 *       el 22 de febrero de 2017.
 *
 * IPP:   Sección 4.9.2 (pp. 186 and ff.)
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "my_rand.h"
#include "timer.h"

/* Los ints aleatorias son inferiores a MAX_KEY */
const int MAX_KEY = 100000000;

/* Valores de retorno de Advance_ptrs */
const int IN_LIST = 1;
const int EMPTY_LIST = -1;
const int END_OF_LIST = 0;

/* Estructura para nodos de la lista */
struct list_node_s {
   int    data;
   pthread_mutex_t mutex;
   struct list_node_s* next;
};

/* Variables compartidas */
struct list_node_s* head = NULL;  
pthread_mutex_t head_mutex;
int         thread_count;
int         total_ops;
double      insert_percent;
double      search_percent;
double      delete_percent;
pthread_mutex_t count_mutex;
int         member_total=0, insert_total=0, delete_total=0;

/* Setup y cleanup */
void        Usage(char* prog_name);
void        Get_input(int* inserts_in_main_p);

/* Función de Thread */
void*       Thread_work(void* rank);

/* Lista de operaciones */
void        Init_ptrs(struct list_node_s** curr_pp, 
      struct list_node_s** pred_pp);
int         Advance_ptrs(struct list_node_s** curr_pp, 
      struct list_node_s** pred_pp);
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
   thread_count = strtol(argv[1], NULL, 10);

   Get_input(&inserts_in_main);

   /* Intenta insertar keys inserts_in_main, pero abandona despues */
   /* 2*inserts_in_main intentos.                                  */
   i = attempts = 0;
   pthread_mutex_init(&head_mutex, NULL);
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
   pthread_mutex_destroy(&head_mutex);
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
/* Función  :  Init_ptrs
 * Propósito:  Inicialice los punteros pred y curr antes de iniciar 
 *             la búsqueda realizada por Insertar o Eliminar
 */
void Init_ptrs(struct list_node_s** curr_pp, struct list_node_s** pred_pp) {
   *pred_pp = NULL;
   pthread_mutex_lock(&head_mutex);
   *curr_pp = head;
   if (*curr_pp != NULL)
      pthread_mutex_lock(&((*curr_pp)->mutex));
// pthread_mutex_unlock(&head_mutex);
      
}  /* Init_ptrs */

/*-----------------------------------------------------------------*/
/* Función  :  Advance_ptrs
 * Propósito:  Avanzar el par de punteros pred y curr durante 
 *             Insertar o Eliminar
 * Suposición: El thread de llamada ya mantiene los bloqueos de los 
 *             nodos a los que hacen referencia curr_p y pred_p
 */
int Advance_ptrs(struct list_node_s** curr_pp, struct list_node_s** pred_pp) {
   int rv = IN_LIST;
   struct list_node_s* curr_p = *curr_pp;
   struct list_node_s* pred_p = *pred_pp;

   if (curr_p == NULL) {
      if (pred_p == NULL) {
         /* A la cabeza de la lista */
         pthread_mutex_unlock(&head_mutex);
         return EMPTY_LIST;
       } else {  /* No encabeza la lista */
         return END_OF_LIST;
       }
   } else { // *curr_pp != NULL
      if (curr_p->next != NULL)
         pthread_mutex_lock(&(curr_p->next->mutex));
      else
         rv = END_OF_LIST;
      if (pred_p != NULL)
         pthread_mutex_unlock(&(pred_p->mutex));
      else
         pthread_mutex_unlock(&head_mutex);
      *pred_pp = curr_p;
      *curr_pp = curr_p->next;
      return rv;
   }
}  /* Advance_ptrs */


/*-----------------------------------------------------------------*/
/* Inserta el valor en la ubicación numérica correcta en la lista */
/* Si el valor no está en la lista, devuelve 1, de lo contrario, devuelve 0 */
int Insert(int value) {
   struct list_node_s* curr;
   struct list_node_s* pred;
   struct list_node_s* temp;
   int rv = 1;

   Init_ptrs(&curr, &pred);
   
   while (curr != NULL && curr->data < value) {
      Advance_ptrs(&curr, &pred);
   }

   if (curr == NULL || curr->data > value) {
#     ifdef DEBUG
      printf("Inserting %d\n", value);
#     endif
      temp = malloc(sizeof(struct list_node_s));
      pthread_mutex_init(&(temp->mutex), NULL);
      temp->data = value;
      temp->next = curr;
      if (curr != NULL) 
         pthread_mutex_unlock(&(curr->mutex));
      if (pred == NULL) {
         // Insertar en el head de la lista
         head = temp;
         pthread_mutex_unlock(&head_mutex);
      } else {
         pred->next = temp;
         pthread_mutex_unlock(&(pred->mutex));
      }
   } else { /* valor en la lista  */
      if (curr != NULL) 
         pthread_mutex_unlock(&(curr->mutex));
      if (pred != NULL)
         pthread_mutex_unlock(&(pred->mutex));
      else
         pthread_mutex_unlock(&head_mutex);
      rv = 0;
   }

   return rv;
}  /* Insertar */

/*-----------------------------------------------------------------*/
/* No usa locks: no se puede ejecutar con los otros subprocesos */
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
   struct list_node_s *temp, *old_temp;

   pthread_mutex_lock(&head_mutex);
   temp = head;
   if (temp != NULL) pthread_mutex_lock(&(temp->mutex));
   pthread_mutex_unlock(&head_mutex);
   while (temp != NULL && temp->data < value) {
      if (temp->next != NULL) 
         pthread_mutex_lock(&(temp->next->mutex));
      old_temp = temp;
      temp = temp->next;
      pthread_mutex_unlock(&(old_temp->mutex));
   }

   if (temp == NULL || temp->data > value) {
#     ifdef DEBUG
      printf("%d no esta en la lista\n", value);
#     endif
      if (temp != NULL) 
         pthread_mutex_unlock(&(temp->mutex));
      return 0;
   } else { /* temp != NULL && temp->data <= value */
#     ifdef DEBUG
      printf("%d esta en la lista\n", value);
#     endif
      pthread_mutex_unlock(&(temp->mutex));
      return 1;
   }
}  /* Es miembro */

/*-----------------------------------------------------------------*/
/* Elimina valor de la lista */
/* Si el valor está en la lista, devuelve 1, de lo contrario, devuelve 0 */
int Delete(int value) {
   struct list_node_s* curr;
   struct list_node_s* pred;
   int rv = 1;

   Init_ptrs(&curr, &pred);

   /* Encuentra valor */
   while (curr != NULL && curr->data < value) {
      Advance_ptrs(&curr, &pred);
   }
   
   if (curr != NULL && curr->data == value) {
      if (pred == NULL) { /* primer elemento de la lista */
         head = curr->next;
#        ifdef DEBUG
         printf("Liberando %d\n", value);
#        endif
         pthread_mutex_unlock(&head_mutex);
         pthread_mutex_unlock(&(curr->mutex));
         pthread_mutex_destroy(&(curr->mutex));
         free(curr);
      } else { /* pred != NULL */
         pred->next = curr->next;
         pthread_mutex_unlock(&(pred->mutex));
#        ifdef DEBUG
         printf("Liberando %d\n", value);
#        endif
         pthread_mutex_unlock(&(curr->mutex));
         pthread_mutex_destroy(&(curr->mutex));
         free(curr);
      }
   } else { /* No en lista */
      if (pred != NULL)
         pthread_mutex_unlock(&(pred->mutex));
      if (curr != NULL)
         pthread_mutex_unlock(&(curr->mutex));
      if (curr == head)
         pthread_mutex_unlock(&head_mutex);
      rv = 0;
   }

   return rv;
}  /* Eliminar */

/*-----------------------------------------------------------------*/
/* No usa locks. Solo se puede ejecutar cuando ningún otro thread 
 * está accediendo a la lista.
 */
void Free_list(void) {
   struct list_node_s* current;
   struct list_node_s* following;

   if (Is_empty()) return;
   current = head; 
   following = current->next;
   while (following != NULL) {
#     ifdef DEBUG
      printf("Liberando %d\n", current->data);
#     endif
      free(current);
      current = following;
      following = current->next;
   }
#  ifdef DEBUG
   printf("Liberando %d\n", current->data);
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
#        ifdef DEBUG
         printf("Thread %ld > Buscando %d\n", my_rank, val);
#        endif
         Member(val);
         my_member++;
      } else if (which_op < search_percent + insert_percent) {
#        ifdef DEBUG
         printf("Thread %ld > Intentando insertar %d\n", my_rank, val);
#        endif
         Insert(val);
         my_insert++;
      } else { /* Eliminar */
#        ifdef DEBUG
         printf("Thread %ld > Intentando eliminar %d\n", my_rank, val);
#        endif
         Delete(val);
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
