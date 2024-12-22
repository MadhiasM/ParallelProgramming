#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

int main(int argc, char *argv[]) {

    int n = 10005;
// warmup
#pragma omp parallel
    if (omp_get_thread_num() == 0)
        printf("I have %d threads \n", omp_get_num_threads());

    gPrimesFound = 0;

    FindPrimes(3, n);
    printf("Found %d primes \n", gPrimesFound);

    return 0;
}
