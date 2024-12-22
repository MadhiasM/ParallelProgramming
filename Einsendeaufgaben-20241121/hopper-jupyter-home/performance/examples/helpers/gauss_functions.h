#ifndef GAUSS_FUNCTIONS_H
#define GAUSS_FUNCTIONS_H
#include <math.h>
#include <stdlib.h>

// Finds the row with the maximum value in column k (pivot column).
// @param A, reference to the input matrix.
// @param n matrix size.
// @param k column index.
int find_max_k(double *A, int n, int m, int k) {
    int maxIdx = k;
    for (int l = k; l < n; l++) {
        if (fabs(A[k * m + k]) < fabs(A[l * m + k])) {
            maxIdx = l;
        }
    }
    return maxIdx;
}

// Swaps the rows i and j starting from column k (there are only zeros to the
// left of k) of the input matrix A.
// @param A, reference to the input matrix.
// @param i,j Indices of the rows to be swapped.
// @param k Column index from which the entries in rows i and j are to be
// swapped.
// @param n Matrix size.
void swap(double *A, int n, int m, int i, int j, int k) {
    double temp;
    if (i != j) {
        for (int l = k; l < m; l++) {
            temp = A[i * m + l];
            A[i * m + l] = A[j * m + l];
            A[j * m + l] = temp;
        }
    }
};

// Solves the system of equations Ax = b by back calculation (see above).
// @param A, b Reference to the input matrix and vector.
// @param x Reference to the solution vector.
// @param n Matrix size.
void backsub(double *A, int n, int m, double *x) {
    for (int i = n - 1; i >= 0; i--) {
        x[i] = A[i * m + m - 1];
        for (int j = i + 1; j < m - 1; j++) {
            x[i] -= A[i * m + j] * x[j];
        }
        x[i] = x[i] / A[i * m + i];
    }
}

// Calculates L2 norm ||Ax-b|| to check if the vector x satisfies the equation
// Ax = и.
// @param A, b Reference to the input matrix and vector.
// @param x Reference to the solution vector.
// @param n Matrix size.
double check(double *A, int n, int m, double *x) {
    double *result_vector = (double *)malloc(sizeof(double) * n);
    double row_sum;

    for (int j = 0; j < n; j++) {
        row_sum = 0;
        for (int k = 0; k < m - 1; k++) {
            row_sum += A[j * m + k] * x[k];
        }
        result_vector[j] = row_sum;
    }

    double sum_of_squares = 0;
    double residual;
    for (int i = 0; i < n; i++) {
        // Vector b is located at positions i*m+m-1 of matrix A
        // printf("x: %f, b %f\n", result_vector[i], A[i*m+m-1]);
        residual = result_vector[i] - A[i * m + m - 1];
        sum_of_squares += residual * residual;
    }
    sum_of_squares = sqrt(sum_of_squares);
    return sum_of_squares;
}

#endif
