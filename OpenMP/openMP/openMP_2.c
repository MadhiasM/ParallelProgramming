#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 8192
#define MAX_VALUE 1024

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
        vector[i] = rand() % MAX_VALUE;
    }
    return vector;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}


int main(int argc, char *argv[]) {
    int N = ARRAY_SIZE;
    double *a = init_random_vector(N);
    double *b = init_random_vector(N);
    double *c = init_random_vector(N);

    #pragma omp parallel
    {
        int n_per_thread = N / omp_get_num_threads();
        if (N % omp_get_num_threads() != 0) {
            n_per_thread += 1;
        }
        int start = n_per_thread * omp_get_thread_num();
        int end = min(N, start + n_per_thread);

        for (int i = start; i < end; i++) {
            c[i] = a[i] + b[i];
            printf("%.0f = %.0f + %.0f\n", c[i], a[i], b[i]);
        }
        printf("Threads: %d\n", omp_get_num_threads());
    }
    return 0;
};
