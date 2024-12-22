#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// IDLE loop to simulate a operation of variable time that is not predictable
// Note that this will likely be compiled out since incr is not used in any other part of the program. Use -O0 to disable optimazation during compilation
void idleloop(int range) {
    int incr = 0;
    for (int l = 0; l < range; l++) {
        incr++;
    }
}

double* init_random_matrix(int n) {
    double *matrix = (double*)malloc(sizeof(double) * n * n);

    // Fehlerbehandlung für malloc
    if (matrix == NULL) {
        fprintf(stderr, "Fehler bei der Speicherallokation\n");
        exit(1);  // Programm beenden
    }

    for (int i=0; i < n; i++) {
        for (int j=0; j < n; j++) {
            matrix[i*n+j] = rand();
        }
    }
    return matrix;
}

double* init_empty_vector(int n) {
    double *vector = (double*)malloc(sizeof(double) * n);
    for (int i = 0; i < n; i++) {
        vector[i] = 0;
    }
    // Fehlerbehandlung für malloc
    if (vector == NULL) {
        fprintf(stderr, "Fehler bei der Speicherallokation\n");
        exit(1);  // Programm beenden
    }
    return vector;
}

double* init_random_vector(int n) {
    double *vector = init_empty_vector(n);

    for (int i=0; i < n; i++) {
        vector[i] = rand();
    }
    return vector;
}

// Fast
void MatrixVectorMultiplication(double *A, double *b, double *c, int n) {
    double sum;
    # pragma omp parallel for private(sum) shared(n, A, b, c) default(none) schedule(dynamic, 1)
    for (int i = 0; i < n; i++) {
        sum = 0;
        for (int j = 0; j < n; j++) {
            double prod = A[i * n + j] * b[j];
            sum += prod;
            idleloop(prod);
        }
        c[i] = sum;
    }
}


int main() {
    // time
    int num_trials = 10;
    long t1, t2;
    double elapsed_time;

    // Matrix Vector
    int lengths[] = {1024, 2048, 8192};
    //int length_min = 1024;
    //int length_max = 8192;

    size_t length = sizeof(lengths) / sizeof(lengths[0]);
    int n;

    #pragma omp parallel num_threads(4)
    {
        if (omp_get_thread_num() == 0)
            printf("I have %d threads \n", omp_get_num_threads());
    }
    for (int k = 0; k < length; k++) {
        n = lengths[k];
        double *A = init_random_matrix(n);
        double *b = init_random_vector(n);
        double *c = init_empty_vector(n);

        t1 = clock();
        for (int l = 0; l < num_trials; l++) {
            MatrixVectorMultiplication (A, b, c, n);
        }
        t2 = clock();
        elapsed_time = (double)(t2 - t1);
        printf("n: %d, time: %lf seconds \n", n, elapsed_time/1000000);

        free(A);
        free(b);
        free(c);
    }
}
