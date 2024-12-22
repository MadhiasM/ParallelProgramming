#include <stdio.h>
#include <omp.h>

#define N 4194304 // 2^20 2^21 2^22 (1048576 2097152 4194304)
#define E_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062

double CalcPi(int n) {
    double delta_x = 1.0f / n;
    double x_i;
    double pi = 0.0;

    #pragma omp parallel for default(none) private(x_i) shared(n, delta_x) reduction(+:pi)
    for (int i = 0; i < n; i++) {
        x_i = i*delta_x;
        pi += (4.0f / (1 + x_i*x_i)) * delta_x;
    }
    return pi;
}


int main() {
    double pi = CalcPi(N);
    double accuracy = (pi - E_PI)/E_PI * 100;

    printf("π ≈ %.10f (approximation)\n", pi);
    printf("π = %.10f (hardcoded)\n", E_PI);
    printf("Accuracy = %.10f%%\n", accuracy);
    return 0;
}
