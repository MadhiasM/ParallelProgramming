#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

/*
a) KGV und Primzahlenberechnung über Sections in 2 Threads parallelisieren
b) Statt KGV und Primzahlen zu parallelisieren, sollen sie sequentiell berechnet werden, aber die Schleifen
innerhalb jeweils die Schleifen parallelisieren
*/

struct zahlen {
    int a;
    int b;
    int kgv;
    int prime[2];
};

struct zahlen *liste;

int TestForPrime(int val) {
    int limit, factor = 3;

    limit = (long)(sqrtf((float)val) + 0.5f);
    while ((factor <= limit) && (val % factor))
        factor++;

    return (factor > limit);
}

int ggT(int zahl1, int zahl2) {
    if (zahl2 == 0) {
        return zahl1;
    }

    return ggT(zahl2, zahl1 % zahl2);
}

int kgV(int zahl1, int zahl2) {
    return (zahl1 * zahl2) / ggT(zahl1, zahl2);
}

void init(int n) {

    for (int i = 0; i < n; i++) {

        liste[i].a = rand() % 100000;
        liste[i].b = rand() % 100000;
    }
}

void berechne_kgv(int n) {
    for (int i = 0; i < n; i++) {

        liste[i].kgv = kgV(liste[i].a, liste[i].b);
    }
}

void berechne_prime(int n) {
    for (int i = 0; i < n; i++) {
        liste[i].prime[0] = TestForPrime(liste[i].a);
        liste[i].prime[1] = TestForPrime(liste[i].b);
    }
}

void compute(int n) {
    berechne_kgv(n);
    berechne_prime(n);
}

int main(int argc, char *argv[]) {
    double before, after;
    int n = 1024;
#pragma omp parallel
    if (omp_get_thread_num() == 0)
        printf("I have %d threads \n", omp_get_num_threads());
    liste = (struct zahlen *)malloc(n * sizeof(struct zahlen));
    init(n);
    before = omp_get_wtime();
    compute(n);
    after = omp_get_wtime();
    printf("Time: %.8f\n", after - before);
    return 0;
}
