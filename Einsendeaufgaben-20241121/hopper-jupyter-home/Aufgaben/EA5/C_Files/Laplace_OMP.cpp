#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#pragma cling load("libomp.so")
//Konstanteninitialisierung
#define MAX_ITER 100
#define EPS 1.0e-4
#define init_rand_val 30
#define init_val 0

double one_jacobi_iteration(double *x, double* xnew, int maxn)
{
double diffnorm=0;
 for(int i=1; i<maxn-1; i++){
        for(int j=1; j<maxn-1; j++){
            xnew[i*maxn+j] = 0.25*(x[i*maxn+j+1] + x[i*maxn+j-1] + 
                                         x[(i+1)*maxn+j] + x[(i-1)*maxn+j]);
            diffnorm +=  (xnew[i*maxn+j] - x[i*maxn+j])*(xnew[i*maxn+j] - x[i*maxn+j]);
        }
    }
    return sqrt(diffnorm);
}

void apply_boundary(double *x, double* xnew, int maxn){

     for(int i=1; i<maxn-1; i++){
         xnew[i]=x[i+(maxn-2)*maxn];
         xnew[i+(maxn-1)*maxn]=xnew[i+maxn];
     }
    for(int i=0; i<maxn; i++){
         xnew[i*maxn]=x[i*maxn+maxn-2];
         xnew[i*maxn+(maxn-1)]=xnew[i*maxn+1];
     }
}

// start_main
// MODIFICATION START main open
int main(int argc, char *argv[]) {
// MODIFICATION END main open
double *x = NULL;
double *xnew = NULL;
int itcnt=0;
double diffnorm;
int n = 16;
if(x!=NULL) free(x);
if(xnew!=NULL) free(xnew);
x = (double*)calloc(n*n, sizeof(double)); 
xnew = (double*)calloc(n*n, sizeof(double));
    for (int i=1; i<n-1; i++){ 
    //Initlisierung des linken und des rechten Elementen des lokalen Gitters in der Zeile i
        x[i*n] = x[i*n+n-1] = init_rand_val;
        for (int j=1; j<n-1; j++){
            //Initialisierung der inneren Elementen des lokalen Gitters
            x[i*n+j] = init_val;
    }
    }
#pragma omp parallel
if(omp_get_thread_num()==0)
    printf("run with %d threads \n", omp_get_num_threads());

//Initialisierung der ersten und der letzten Zeilen des lokalen Gitters

    for (int j=0; j<n; j++) {
        x[j] = init_rand_val;
        x[(n-1)*n+j] = init_rand_val;
   }

do{
    itcnt++;
    diffnorm = one_jacobi_iteration(x,xnew,n);
    apply_boundary(x, xnew,n);
    double *tmp = x;
    x= xnew;
    xnew = tmp;
    printf("Diffnorm %f \n" , diffnorm);
}while(diffnorm > EPS && itcnt < MAX_ITER);


free(x);
free(xnew);
// end_main
// MODIFICATION START main close
return 0;
}
// MODIFICATION END main close
