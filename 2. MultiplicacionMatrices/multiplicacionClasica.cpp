//  Created by Renzo Alessandro on 26/03/21.
//  Copyright © 2021 RenzoAlessandro. All rights reserved.

#include <iostream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int get_random(int low, int high) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distribution(low, high);
  return distribution(gen);
}

void print_matrix(double** Matrix, int fila, int columna){
    for(int i=0; i<fila; ++i){
        for(int j=0; j<columna; ++j){
            std::cout<<Matrix[i][j]<<" ";
        }
        std::cout<<std::endl;
    }
}

void simple_multiplication(int k, int m, int n, double** A, double** B, double** C){
    for(int i=0; i<k; ++i){
        for(int j=0; j<n; ++j){
            for(int z=0; z<m; ++z){
                C[i][j] += A[i][z] * B[z][j];
            }
        }
    }
}
 
int main()
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    int n;
    double** A;
    double** B;
    double** C;
    int i=0;
    int j=0;
    std::cout<<"Ingrese la dimensión de Matriz (n): "; std::cin>>n;
    /* Asignar memoria para las matrices */
     
    /************     Matriz A      ************/
    A =(double **)malloc(n*sizeof(double *));
    A[0] = (double *)malloc(n*n*sizeof(double));
    if(!A)
    {
        printf("La memoria falló. \n");
        exit(1);
    }
    for(i=1; i<n; i++)
    {
        A[i] = A[0]+i*n;
        if (!A[i])
        {
            printf("La memoria falló. \n");
            exit(1);
        }
    }
 
    /************     Matriz B      ************/
    B =(double **)malloc(n*sizeof(double *));
    B[0] = (double *)malloc(n*n*sizeof(double));
    if(!B)
    {
        printf("La memoria falló. \n");
        exit(1);
    }
    for(i=1; i<n; i++)
    {
        B[i] = B[0]+i*n;
        if (!B[i])
        {
            printf("La memoria falló. \n");
            exit(1);
        }
    }
 
    //************     Matriz B      ************/
    C =(double **)malloc(n*sizeof(double *));
    C[0] = (double *)malloc(n*n*sizeof(double));
    if(!C)
    {
        printf("La memoria falló. \n");
        exit(1);
    }
    for(i=1; i<n; i++)
    {
        C[i] = C[0]+i*n;
        if (!C[i])
        {
            printf("La memoria falló. \n");
            exit(1);
        }
    }
 
    /* Inicializamos la matriz A y B. */
    for(i=0; i<n; i++)
    {
        for(j=0; j<n; j++)
        {
            A[i][j] = get_random(1, 100);
            B[i][j] = get_random(1, 100);
        }
    }
    
    /*  
        Multiplicación principal 
        C[i][j] = A[i][0] * B[0][j] + A[i][1] * B[1][j] + A[i][2] * B[2][j] + ... + A[i][m-1] * B[m-1][j] 
    */
    start = std::chrono::high_resolution_clock::now();
    simple_multiplication(n,n,n,A,B,C);
    end = std::chrono::high_resolution_clock::now();

    /*  Imprimimos las matrices A, B y C  */
    // std::cout<<"\tMatriz A"<<std::endl;
    // print_matrix(A, n, n);
    // std::cout<<"\tMatriz B"<<std::endl;
    // print_matrix(B, n, n);
    // std::cout<<"\tMatriz C"<<std::endl;
    // print_matrix(C, n, n);
    
    /* Desasignar memoria para las matrices  */
    free(A[0]);
    free(A);
    free(B[0]);
    free(B);
    free(C[0]);
    free(C);

    long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "\tTiempo: " + std::to_string(duration) + " milliseconds.\n" << std::endl;
    return 0;
}
