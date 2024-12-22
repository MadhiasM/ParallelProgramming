#include <cstdio>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXN 2147483647

double f(double x) { return 4.0 / (1 + x * x); }

double CalcPi(int n) {
    // berechne die Fläche der Rechtecke
    if (n <= 0 || n > MAXN) {
        return 0.0;
    }
    const double fH = 1.0 / (double)n;

    // initialisiere die Summe der Funktionswerte
    double fSum = 0.0;
    double fX;
    // int i;

    for (int i = 0; i < n; i += 1) {
        fX = fH * ((double)i + 0.5);
        fSum += f(fX);
    }
    return fH * fSum;
}

// start_main
// MODIFICATION START main open
int main(int argc, char *argv[]) {
    // MODIFICATION END main open
    int n = 32;
    double pi = CalcPi(n);
    printf("Pi is %f \n", pi);
    // end_main
    // MODIFICATION START main close
    return 0;
}
// MODIFICATION END main close
