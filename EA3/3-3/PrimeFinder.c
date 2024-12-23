#include <math.h>
#include <omp.h>
#include <stdio.h>

static int gPrimesFound = 0;
long globalPrimes[1000000];

int TestForPrime(int val) {
    int limit, factor = 3;

    limit = (long)(sqrtf((float)val) + 0.5f);
    while ((factor <= limit) && (val % factor))
        factor++;

    return (factor > limit);
}

void FindPrimes(int start, int end) {
    int i;

    for (i = start; i <= end; i += 2) {
        if (TestForPrime(i)) {
            globalPrimes[gPrimesFound++] = i;
        }
    }
}

void FindPrimesStatic(int start, int end) {
    int i;

    #pragma omp parallel for schedule(static,5) default(none) shared(end, start, gPrimesFound, globalPrimes) private(i)
    for (i = start; i <= end; i += 2) {
        if (TestForPrime(i)) {
            #pragma omp critical
            globalPrimes[gPrimesFound++] = i;
        }
    }
}

void FindPrimesDynamic(int start, int end) {
    int i;

    #pragma omp parallel for schedule(dynamic,5) default(none) shared(end, start, gPrimesFound, globalPrimes) private(i)
    for (i = start; i <= end; i += 2) {
        if (TestForPrime(i)) {
            #pragma omp critical
            globalPrimes[gPrimesFound++] = i;
        }
    }
}

int main(int argc, char *argv[]) {
    int n = 8388609; // 2097153, 4194305, 8388609
    double t0, t1;

// warmup
    #pragma omp parallel num_threads(8)
    if (omp_get_thread_num() == 0)
        printf("I have %d threads \n", omp_get_num_threads());

    gPrimesFound = 0;
    t0 = omp_get_wtime();
    FindPrimes(3, n);
    t1 = omp_get_wtime();
    printf("Serial:\nFound %d primes\nIn %.8f seconds\n", gPrimesFound, t1-t0);

    gPrimesFound = 0;
    t0 = omp_get_wtime();
    FindPrimesStatic(3, n);
    t1 = omp_get_wtime();
    printf("Parallel Static:\nFound %d primes\nIn %.8f seconds\n", gPrimesFound, t1-t0);

    gPrimesFound = 0;
    t0 = omp_get_wtime();
    FindPrimesDynamic(3, n);
    t1 = omp_get_wtime();
    printf("Parallel Dynamic:\nFound %d primes\nIn %.8f seconds\n", gPrimesFound, t1-t0);

    return 0;
}
