#include <stdio.h>
#include <stdlib.h>
double *a, *b, *c;

void vector_add(double *a, double *b, double *c, int n) {
  for (int i = 0; i < n; i++) {
    c[i] = a[i] + b[i];
  }
}
void init(int n) {

  a = (double *)calloc(n, sizeof(double));
  b = (double *)calloc(n, sizeof(double));
  c = (double *)calloc(n, sizeof(double));

  for (int i = 0; i < n; i++) {
    a[i] = rand() % 10;
    b[i] = rand() % 10;
    c[i] = 0;
  }
}

void cleanup() {
  if (a) {
    free(a);
    a = NULL;
  }
  if (b) {
    free(b);
    b = NULL;
  }
  if (c) {
    free(c);
    c = NULL;
  }
}

int main() {
  int n = 2 << 20;
  cleanup();
  init(n);
  vector_add(a, b, c, n);
  cleanup();
}
