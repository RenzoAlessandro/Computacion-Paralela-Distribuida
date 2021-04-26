/* Archivo:     timer.h
 *
 * Propósito:  Defina una macro que devuelva el número de segundos que 
 *             han transcurrido desde algún momento en el pasado. 
 *             El temporizador debe devolver tiempos con una precisión 
 *             de microsegundos.
 *
 * Nota:       El argumento pasado a la macro GET_TIME debe ser un doble, 
 *             *no* un puntero a un doble.
 *
 * Ejemplo:  
 *    #include "timer.h"
 *    . . .
 *    double start, finish, elapsed;
 *    . . .
 *    GET_TIME(start);
 *    . . .
 *    Code to be timed
 *    . . .
 *    GET_TIME(finish);
 *    elapsed = finish - start;
 *    printf("The code to be timed took %e seconds\n", elapsed);
 *
 * IPP:  Sección 3.6.1 (págs. 121 y sigs.) y Sección 6.1.2 (págs. 273 y sigs.)
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>

/* El argumento now debería ser un doble (no un puntero a un doble)*/
#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

#endif