#include<iostream>
#include <random>
#define MAX 1000

int get_random(int low, int high) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distribution(low, high);
  return distribution(gen);
}

int main()
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    double A[MAX][MAX], x[MAX], y[MAX];

    /*  Inicializamos A y x con valores aleatorios  */
    for (int i = 0; i < MAX; i++){
        for (int j = 0; j < MAX; j++){
            A[i][j] = get_random(1, MAX);
            //std::cout<<A[i][j]<<std::endl;
        }
    }
    for (int j = 0; j < MAX; j++){
        x[j] = get_random(1, MAX);
        //std::cout<<x[j]<<std::endl;
    }

    /*  Primer Par de loops  */
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < MAX; i++){
        for (int j = 0; j < MAX; j++){
            y[i] = A[i][j]*x[j];
        }
    }
    end = std::chrono::high_resolution_clock::now();

    /*  Segundo Par de loops  */
    // start = std::chrono::high_resolution_clock::now();
    // for (int j = 0; j < MAX; j++){
    //     for (int i = 0; i < MAX; i++){
    //         y[i] = A[i][j]*x[j];
    //     }
    // }
    // end = std::chrono::high_resolution_clock::now();


as

    long long duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "\tDuración: " + std::to_string(duration) + "s\n" << std::endl;
    return 0;
}

