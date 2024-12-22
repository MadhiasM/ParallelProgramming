#include <stdio.h>

#define SIZE 10001

int main() {
    // Statische Zuweisung von Speicher für SIZE floats
    float arr[SIZE];

    arr[0] = 1.0f;
    for (int i = 1; i < SIZE; i++) {
        arr[i] = i * 10E-9f; // Werte zuweisen
    }
    // Beispiel: Zugriff auf Array-Werte
    printf("Erster Wert: %f\n", arr[0]);
    printf("Letzter Wert: %f\n", arr[999]);

    return 0;
}
