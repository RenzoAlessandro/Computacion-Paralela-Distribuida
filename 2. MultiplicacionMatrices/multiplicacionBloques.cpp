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
 
void block_multiplication(int n, double** a, double** b, double** c){
    int bi, bj, bk, i, j, k;
    bi = bj = bk = i =  j = k = 0;
    int blockSize=10; 
    
    for(bi=0; bi<n; bi+=blockSize){
        for(bj=0; bj<n; bj+=blockSize){
            for(bk=0; bk<n; bk+=blockSize){
                for(i=0; i<blockSize; i++){
                    for(j=0; j<blockSize; j++){
                        for(k=0; k<blockSize; k++){
                            c[bi+i][bj+j] += a[bi+i][bk+k]*b[bk+k][bj+j];
                        }
                    }
                }
            }
        }
    }
}
 
int main(void)
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    int n;
    double** A;
    double** B;
    double** C;
    int i=0;
    int j=0;
    std::cout<<"Ingrese la dimensión de Matriz (n): "; std::cin>>n;
    // Asignar memoria para las matrices
     
    ///////////////////// Matrix A //////////////////////////
     
    A =(double **)malloc(n*sizeof(double *));
    A[0] = (double *)malloc(n*n*sizeof(double));
    if(!A)
    {
        printf("memory failed \n");
        exit(1);
    }
    for(i=1; i<n; i++)
    {
        A[i] = A[0]+i*n;
        if (!A[i])
        {
            printf("memory failed \n");
            exit(1);
        }
    }
 
    ///////////////////// Matrix B //////////////////////////
    B =(double **)malloc(n*sizeof(double *));
    B[0] = (double *)malloc(n*n*sizeof(double));
    if(!B)
    {
        printf("memory failed \n");
        exit(1);
    }
    for(i=1; i<n; i++)
    {
        B[i] = B[0]+i*n;
        if (!B[i])
        {
            printf("memory failed \n");
            exit(1);
        }
    }
 
    ///////////////////// Matrix C //////////////////////////
    C =(double **)malloc(n*sizeof(double *));
    C[0] = (double *)malloc(n*n*sizeof(double));
    if(!C)
    {
        printf("memory failed \n");
        exit(1);
    }
    for(i=1; i<n; i++)
    {
        C[i] = C[0]+i*n;
        if (!C[i])
        {
            printf("memory failed \n");
            exit(1);
        }
    }
 
    // Inicializamos la matriz A y B
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
        
    */
    start = std::chrono::high_resolution_clock::now();
    block_multiplication(n,A,B,C);
    end = std::chrono::high_resolution_clock::now();

    /*  Imprimimos las matrices A, B y C  */
    // std::cout<<"\tMatriz A"<<std::endl;
    // print_matrix(A, n, n);
    // std::cout<<"\tMatriz B"<<std::endl;
    // print_matrix(B, n, n);
    // std::cout<<"\tMatriz C"<<std::endl;
    // print_matrix(C, n, n);
     
    // Desasignar memoria para las matrices
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