#include <stdio.h>

#define N 4194304 // 2^20 2^21 2^22 (1048576 2097152 4194304)
#define E_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062

double CalcPi(int n) {
    double delta_x = 1.0f / n;
    double x_i = 0.0;
    double pi = 0.0;
    for (int i = 0; i < n; i++) {
        pi += (4.0f / (1 + x_i*x_i)) * delta_x;
        // This approach will not work in parallel programming since it relies on all prrevious steps (lower iterations of i) have been performed
        // However, for serial apporach, this is faster than using x_1 = i*delta_x for each iteration
        x_i += delta_x;     }
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
