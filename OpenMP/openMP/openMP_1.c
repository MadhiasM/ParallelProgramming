#include <omp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("Master-Thread\n");
    #pragma omp parallel
    { printf("Slave-Thread %d\n", omp_get_thread_num()); }
    printf("Wieder Master-Thread\n");
    return 0;
}
