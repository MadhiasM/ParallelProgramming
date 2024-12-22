#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**Berechnet das Produkt c = Ab, wobei A eine quadratische Matrix der Größe nxn
   und b ein Vektor der Größe nx1 sind.

    @param A, b Verweise auf die Eingabedaten.
    @param c Verweis auf den Ausgabenvektor.
    @param n die Anzahl Zeilen und Spalten in der Eingabematrix.
    @param nThreads die Anzahl Threads.
    @param anzC die Anzahl Chunks beim Scheduling.
    **/
void MatrixVectorMult(double *A, double *b, double *c, int n) {
    double sum;
    /*Matrix-Vector Multiplikation*/
    for (int i = 0; i < n; i++) {
        sum = 0;
        for (int j = 0; j < n; j++) {
            double prod = A[i * n + j] * b[j];
            sum += prod;
        }
        c[i] = sum;
    }
}

void init_matrix(double **M, int n, int m, bool zeros) {
    double *mat = (double *)malloc(n * m * sizeof(double));
    if (zeros)
        memset(mat, 0x0, n * m * sizeof(double));
    else {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                mat[i * m + j] = 5.0 - (double)(rand() % 20) / 10;
            }
        }
    }
    *M = mat;
}

int main(int argc, char *argv[]) {
    int n = 2048;
    double *A;
    double *b;
    double *c = (double *)malloc(n * sizeof(double));
    init_matrix(&A, n, n, false);
    init_matrix(&b, n, 1, false);
#pragma omp parallel num_threads(4)
    {
        if (omp_get_thread_num() == 0)
            printf("I have %d threads \n", omp_get_num_threads());
    }
    MatrixVectorMult(A, b, c, n);

    free(A);
    free(b);
    free(c);

    return 0;
}
