#include <stdio.h>
#include <stdlib.h>
#include <time.h>


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

void MatrixVectorMultiplicatioByRow (double *A, double *b, double *c, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double prod = A[i * n + j] * b[j];
            c[i] += prod;
        }
    }
}
void MatrixVectorMultiplicatioByColumn (double *A, double *b, double *c, int n) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            double prod = A[i * n + j] * b[j];
            c[i] += prod;
        }
    }
}


int main() {
    // time
    int num_trials = 10;
    long t1, t2;
    double elapsed_time;

    // Matrix Vector
    int lengths[] = {1024, 8192};
    int length_min = 1024;
    int length_max = 8192;

    size_t length = sizeof(lengths) / sizeof(lengths[0]);
    int n;


    for (int k = 0; k < length; k++) {
        n = lengths[k];
        double *A = init_random_matrix(n);
        double *b = init_random_vector(n);

        t1 = clock();
        for (int l = 0; l < num_trials; l++) {
            double *c = init_empty_vector(n);
            MatrixVectorMultiplicatioByRow(A, b, c, n);
            free(c);
        }
        t2 = clock();
        elapsed_time = (double)(t2 - t1);
        printf("Rows first, n: %d, time: %lf seconds \n", n, elapsed_time/1000000);

        t1 = clock();
        for (int m = 0; m < num_trials; m++) {
            double *c = init_empty_vector(n);
            MatrixVectorMultiplicatioByColumn(A, b, c, n);
            free(c);
        }
        t2 = clock();
        elapsed_time = (double)(t2 - t1);
        printf("Columns first, n: %d, time: %lf seconds \n", n, elapsed_time/1000000);

        free(A);
        free(b);
    }
}
