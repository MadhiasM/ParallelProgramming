#ifndef MATRIX_FUNCTIONS_H
#define MATRIX_FUNCTIONS_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PRINT_TOTAL_WIDTH "5"

// Initialize a matrix with randomized numbers from the interval [0..9] or with
// zeros.
// @param A_out, reference to the output data.
// @param n,m the number of rows or columns in the output matrix.
// @param zeroM for initialization with zeros.
void init_matrix(double **A_out, int n, int m, bool zeroM) {
    int m_size = n * m;
    const size_t bytes = m_size * sizeof(double);
    *A_out = (double *)malloc(bytes);

    for (int i = 0; i < m_size; i++) {
        if (zeroM) {
            (*A_out)[i] = 0;
        } else {
            (*A_out)[i] = rand() % 100;
        }
    }
};

// Create a copy of the input matrix A_in.
// @param A_in, Reference to the input matrix.
// @param A_out, reference to the output matrix.
// @param n,m the number of rows and columns in the input matrix.
void copy_matrix(const double *A_in, double **A_out, int n, int m) {
    const size_t bytes = n * m * sizeof(double);
    *A_out = (double *)malloc(bytes);
    memcpy((void *)*A_out, (const void *)A_in, bytes);
};

// Returns the input matrix to the console in the form
//
// |a11 a12 ... a1n|
// ...
// |an1 an2 ... ann|
//
// @param A, reference to the input matrix.
// @param n the number of rows in the input matrix.
// @param m the number of columns in the input matrix.
void print_matrix(double *A, int n, int m) {
    for (int i = 0; i < n; i++) {
        printf("|");
        for (int j = 0; j < m; j++) {
            printf("%" PRINT_TOTAL_WIDTH ".2f ", A[i * m + j]);
        }
        printf("|\n");
    }
};

// Returns the input matrix to the console in the form
//
// |a11 a12 ... a1n | b1 |
// ...
// |an1 an2 ... ann | bn |
//
// @param A, reference to the input matrix.
// @param n the number of rows in the input matrix.
// @param m the number of columns in the input matrix.
void print_matrix_vector(double *A, int n, int m) {
    for (int i = 0; i < n; i++) {
        printf("| ");
        for (int j = 0; j < m - 1; j++) {
            printf("%" PRINT_TOTAL_WIDTH ".2f ", A[i * m + j]);
        }
        // vector
        printf("| | %" PRINT_TOTAL_WIDTH ".2f |\n", A[i * m + m - 1]);
    }
}

#endif
