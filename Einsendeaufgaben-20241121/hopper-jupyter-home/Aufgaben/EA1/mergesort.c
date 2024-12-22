#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma cling load("libomp.so")
#define LENGTH 1000000
#define REPEATS 10
#define NUMTHREADS 64

/* Copyright belongs to https://github.com/avcourt, code accessed on
 * https://github.com/avcourt/task-parallelism-omp/blob/master/code/merge_tasks_unnested.c.
 * Code edited by Maruna Derieg. */

void merge(int a[], int size, int temp[]);

void mergesort_serial(int a[], int size, int temp[]);

void mergesort_parallel_omp(int a[], int size, int temp[], int thresh);

void validate_sort(int n, int *data);

int main(int argc, char *argv[]) {

    // initialize data
    int n = LENGTH, repeats = REPEATS, num_threads = NUMTHREADS, thresh = 100,
        size = LENGTH;
    double time, time_min, time_start, time_end;

    // check for passed arguments
    if (argc == 1) {
        printf("No arguments have been passed. Default values are: \n");
        printf("1. Number of threads: %d.\n", num_threads);
        printf("2. Length of array to be sorted: %d.\n", size);
    } else if (argc == 2) {
        num_threads = atoi(argv[1]);
        printf("No arguments for list length has been passed. Default values "
               "are: \n");
        printf("2. Length of array to be sorted: %d.\n", size);
    } else if (argc == 3) {
        num_threads = atoi(argv[1]);
        size = atoi(argv[2]);
    } else {
        printf("Error: Too many arguments have been passed. Defaults are set "
               "to:\n");
        printf("1. Number of threads: %d.\n", num_threads);
        printf("2. Length of array to be sorted: %d.\n", size);
    }

    // execute mergesort algorithm multiple times and save smallest execution
    // time to time file
    for (int iteration = 0; iteration < repeats; iteration++) {
        // generate array of random numbers (seed will be same for each run)
        int *a = malloc(sizeof(int) * size);
        int *temp = malloc(sizeof(int) * size);
        if (a == NULL || temp == NULL) {
            printf("Error: Could not allocate array of size %d\n", size);
            return 1;
        }
        for (int i = 0; i < size; i++) {
            a[i] = rand() % size;
        }
        // measure time while running algorithm
        omp_set_num_threads(num_threads);
        time_start = omp_get_wtime();
#pragma omp parallel
        {
#pragma omp single nowait
            mergesort_parallel_omp(a, size, temp, thresh);
        }
        time_end = omp_get_wtime();
        time = time_end - time_start;
        if (iteration == 0)
            time_min = time;
        else {
            if (time < time_min)
                time_min = time;
        }
        // check result and free data
        validate_sort(size, &a[0]);
        free(a);
    }

    // output
    printf("Smallest execution time of %d runs: %f \n", repeats, time_min);

    return 0;
}

void mergesort_parallel_omp(int a[], int size, int temp[], int thresh) {
    if (size <= thresh) {
        mergesort_serial(a, size, temp);
    } else {
#pragma omp task
        { mergesort_parallel_omp(a, size / 2, temp, thresh); }
#pragma omp task
        {
            mergesort_parallel_omp(a + size / 2, size - size / 2,
                                   temp + size / 2, thresh);
        }
#pragma omp taskwait
        { merge(a, size, temp); }
    }
}

// only called if num_threads = 1
void mergesort_serial(int a[], int size, int temp[]) {
    // Switch to insertion sort for small arrays
    if (size > 1) {
        mergesort_serial(a, size / 2, temp);
        mergesort_serial(a + size / 2, size - size / 2, temp);
        merge(a, size, temp);
    }
}

void merge(int a[], int size, int temp[]) {
    int i1 = 0;
    int i2 = size / 2;
    int tempi = 0;
    while (i1 < size / 2 && i2 < size) {
        if (a[i1] < a[i2]) {
            temp[tempi] = a[i1];
            i1++;
        } else {
            temp[tempi] = a[i2];
            i2++;
        }
        tempi++;
    }
    while (i1 < size / 2) {
        temp[tempi] = a[i1];
        i1++;
        tempi++;
    }
    while (i2 < size) {
        temp[tempi] = a[i2];
        i2++;
        tempi++;
    }
    // Copy sorted temp array into main array, a
    memcpy(a, temp, size * sizeof(int));
}

void validate_sort(int n, int *data) {
    int i;
    for (i = 0; i < n - 1; i++) {
        if (data[i] > data[i + 1]) {
            printf("ERROR: Validate failed\n");
            break;
        }
    }
}
