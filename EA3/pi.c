#include <stdio.h>
#include <omp.h>

#define N 4194304 // 2^20 2^21 2^22 (1048576 2097152 4194304)
#define E_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062
#define NMAX 2147483647 // Max possible value for 32 bit signed integer


#define COLOR_BOLD  "\e[1m"
#define COLOR_OFF   "\e[m"

double CalcPi_serial(int n) {
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

double CalcPi_reduction(int n) {
    double delta_x = 1.0f / n;
    double x_i;
    double pi = 0.0;

    #pragma omp parallel for schedule(static) default(none) private(x_i) shared(n, delta_x) reduction(+:pi)
    for (int i = 0; i < n; i++) {
        x_i = i*delta_x;
        pi += (4.0f / (1 + x_i*x_i)) * delta_x;
    }
    return pi;
}

double CalcPi_critical(int n) {
    if (n <= 0 || n > NMAX) {
        return 0.0;
    }
    double delta_x = 1.0f / n;
    double x_i;
    double pi = 0.0;

    #pragma omp parallel for schedule(static) default(none) private(x_i) shared(n, delta_x, pi)
    for (int i = 0; i < n; i++) {
        x_i = i*delta_x;
        #pragma omp critical
        pi += (4.0f / (1 + x_i*x_i)) * delta_x;
    }
    return pi;
}

double CalcPi_atomic(int n) {
    if (n <= 0 || n > NMAX) {
        return 0.0;
    }
    double delta_x = 1.0f / n;
    double x_i;
    double pi = 0.0;

    #pragma omp parallel for schedule(static) default(none) private(x_i) shared(n, delta_x, pi)
    for (int i = 0; i < n; i++) {
        x_i = i*delta_x;
        #pragma omp atomic
        pi += (4.0f / (1 + x_i*x_i)) * delta_x;
    }
    return pi;
}

float accuracy_pi(float pi_approx) {
    return (pi_approx - E_PI)/E_PI * 100;
}

int main() {
    double pi;
    double accuracy;

    pi = CalcPi_serial(N);
    accuracy = accuracy_pi(pi);

    printf(COLOR_BOLD "Serial:" COLOR_OFF "\n");
    printf("π ≈ %.10f (approximation)\n", pi);
    printf("Accuracy = %.10f%%\n", accuracy);

    pi = CalcPi_reduction(N);
    accuracy = accuracy_pi(pi);

    printf(COLOR_BOLD "Reduction:" COLOR_OFF "\n");
    printf("π ≈ %.10f (approximation)\n", pi);
    printf("Accuracy = %.10f%%\n", accuracy);

    pi = CalcPi_critical(N);
    accuracy = accuracy_pi(pi);

    printf(COLOR_BOLD "Critical:" COLOR_OFF "\n");
    printf("π ≈ %.10f (approximation)\n", pi);
    printf("Accuracy = %.10f%%\n", accuracy);

    pi = CalcPi_atomic(N);
    accuracy = accuracy_pi(pi);

    printf(COLOR_BOLD "Atomic:" COLOR_OFF "\n");
    printf("π ≈ %.10f (approximation)\n", pi);
    printf("Accuracy = %.10f%%\n", accuracy);

    return 0;
}
