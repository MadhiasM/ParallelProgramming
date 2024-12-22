#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "csr_formatter.h" 
#include <functional>
#include <string>
#pragma cling load("libomp.so")
#define ABS(a) ((a) >0 ? (a) :-(a))


double randfrom(double min, double max) 
{
    double range = (max - min); 
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

void SpMV_dynamic(double *data, int *cols, int *rowptr, int n, double *b, double *c){
 
}

void SpMV_static(double *data, int *cols, int *rowptr, int n, double *b, double *c){
 
    
}

//Für den Vergleich die Klassiche Berechnung
void DMV(double *data, int n, int k, double *b, double *c){
    #pragma omp parallel for
  for(int i = 0; i<n; i++){
      double tmp=0;
      for(int j=0; j<k; j++){
          tmp+=data[i*k+j]*b[j];
         
      }
      c[i]=tmp;
  }  
}

std:: string choose_file(int n){
if (n==0)
     return "../data/c-22.mtx";
 if (n==1)
     return "../data/parabolic_fem.mtx";
else if (n==2) 
    return "../data/nxp1.mtx";
else if (n==3)
     return "../data/bundle_adj.mtx";
else if( n==4) 
    return "../data/ss.mtx";
else return "";
}

// start_main
// MODIFICATION START main open
int main(int argc, char *argv[]) {
// MODIFICATION END main open
CSR* Mat;
double t2,t1;
int n = 0;
Mat = assemble_csr_matrix(choose_file(n));
double* b = (double*)malloc(Mat->N*sizeof(double));
double* c = (double*)malloc(Mat->M*sizeof(double));
for(int i=0; i<Mat->N; i++) {
    b[i]=randfrom(-100, 100);   
}
#pragma omp parallel
{
if(omp_get_thread_num()==0)
    printf("run %d threads \n", omp_get_num_threads());

}

SpMV_static(Mat->val, Mat->col_ind, Mat->row_ptr, Mat->N, b, c);

SpMV_dynamic(Mat->val, Mat->col_ind, Mat->row_ptr, Mat->N, b, c);



if(Mat->N < (1<<15)) {
//Achtung, nur bei kleinen Dense Martizen ausführen, für den Vergleich
     double * m=  generate_dense_matrix(Mat);
    double* c2 = (double*)malloc(Mat->M*sizeof(double));
DMV(m, Mat->N, Mat->M, b, c2);
int p =0;
for (int i=0; i<Mat->N; i++) {
   if(ABS(c[i]-c2[i])>1e-4) {printf(" wrong %d %f %f %f\n",i,c[i],c2[i], ABS(c[i]-c2[i]));p++; if(p>20) break; }
}
    if(p==0)
        printf("result is correct\n");

 free(m);
 free(c2);
}
free_CSR(Mat);
free(Mat);
// end_main
// MODIFICATION START main close
return 0;
}
// MODIFICATION END main close
