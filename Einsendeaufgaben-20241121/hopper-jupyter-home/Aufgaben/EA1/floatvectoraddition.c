#include <stdlib.h>
#include <stdio.h>

#define SIZE 10001
//#define DEBUG

void printBinary(unsigned int num) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (num >> i) & 1);

        //if (i % 4 == 0) {
        if (i == 32-1 || i == 32-9) {
            printf(" "); // Gruppiere in Vorzeichen, Exponent und Mantisse
        }
    }
    printf("\n");
    printf("^ ^^^^^^^^ ^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("Sign                              \n");
    printf("  Exponent                        \n");
    printf("           Mantissa              \n");
}


int main() {
    //a) Schreiben Sie ein Programm, welches einen Vektor für n = 10001 float-Werte alloziert. Der erste Wert soll dabei auf 1 initialisiert werden, die übrigen Werte mit 10^-9
    int size = 10001;
    float *arr = (float*)malloc(sizeof(float)*SIZE);
    arr[0] = 1.0;
    for (int i=1; i < SIZE; i++) {
        arr[i]= 10E-9;

        #ifdef DEBUG
            printf("%.10f\n", arr[i]);
        #endif
    }
    //b) Nun addieren Sie die Werte des Vektors in einer Schleife einmal von 0 – (n − 1) zusammen und einmal von (n − 1) – 0, also einmal vorwärts und einmal rückwärts
    float sumup = 0;

    for (int j=0; j < SIZE; j++) {
        sumup += arr[j];
    }

    printf("Sum up:   %.08f\n", sumup);

    float sumdown = 0;
    for (int j=SIZE-1; j >= 0; j--) {
        sumdown += arr[j];
    }
    printf("Sum down: %.08f\n", sumdown);

    printf("Difference: %.08f\n", sumup-sumdown);

    //c) Vergleichen Sie die Ergebnisse. Lassen Sie sich dazu das Ergebnis am besten auf 8 Nachkommastellen genau ausgeben, dies geht z.B. mit printf("%.8f\n",sum). Was fällt ihnen auf? Erläutern Sie ihre Ergebnisse! Welche Probleme glaube sie ergeben sie daraus ggf. für die parallele Programmierung?
    unsigned int *floatAsIntUp = (unsigned int*)&sumup;
    printf("Binary representation of %.08f:\n", sumup);
    printBinary(*floatAsIntUp);

    unsigned int *floatAsIntDown = (unsigned int*)&sumdown;
    printf("Binary representation of %.08f:\n", sumdown);
    printBinary(*floatAsIntDown);

    unsigned int *floatAsInt1 = (unsigned int*)&arr[0];
    printf("Binary representation of %.08f:\n", arr[0]);
    printBinary(*floatAsInt1);

    unsigned int *floatAsInt1Eneg9 = (unsigned int*)&arr[1];
    printf("Binary representation of %.08f:\n", arr[1]);
    printBinary(*floatAsInt1Eneg9);

    return 0;
}
