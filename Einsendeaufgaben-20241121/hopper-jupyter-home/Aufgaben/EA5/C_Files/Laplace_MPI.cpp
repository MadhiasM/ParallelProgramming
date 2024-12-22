
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

//Konstanteninitialisierung
#define MAX_ITER 100
#define EPS 1.0e-4
#define init_rand_val 30
#define init_val 0

double one_jacobi_iteration(double *x, double* xnew, int maxn, int n_local)
{
double diffnorm=0;
 for(int i=1; i<n_local-1; i++){
        for(int j=1; j<maxn-1; j++){
            xnew[i*maxn+j] = 0.25*(x[i*maxn+j+1] + x[i*maxn+j-1] + 
                                         x[(i+1)*maxn+j] + x[(i-1)*maxn+j]);
            diffnorm +=  (xnew[i*maxn+j] - x[i*maxn+j])*(xnew[i*maxn+j] - x[i*maxn+j]);
        }
    }
    return diffnorm;
}

void apply_boundary(double *x, double* xnew, int maxn, int local_n){
     for(int i=0; i<local_n-1; i++){
         xnew[i*maxn]=x[i*maxn+maxn-2];
         xnew[i*maxn+(maxn-1)]=xnew[i*maxn+1];
     }
}

void apply_boundary_single(double *x, double* xnew, int maxn){

     for(int i=1; i<maxn-1; i++){
         xnew[i]=x[i+(maxn-2)*maxn];
         xnew[i+(maxn-1)*maxn]=xnew[i+maxn];
     }
    for(int i=0; i<maxn; i++){
         xnew[i*maxn]=x[i*maxn+maxn-2];
         xnew[i*maxn+(maxn-1)]=xnew[i*maxn+1];
     }
}



int main(int argc, char *argv[]) {

	MPI_Init(NULL, NULL);
	int rank, num_p;


	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_p);

	double *x = NULL;
	double *xnew = NULL;
	double *xall = NULL;
	int itcnt=0;
	double diffnorm;

	int n = 1024;

	//Lokale initialisierung, nur auf Rank 0! 
	if(rank == 0) {
		if(xall!=NULL) free(xall);
		xall = (double*)calloc(n*n, sizeof(double)); 
		for (int i=1; i<n-1; i++){ 
			//Initialisierung des linken und des rechten Elementen des lokalen Gitters in der Zeile i
			xall[i*n] = xall[i*n+n-1] = init_rand_val;
			for (int j=1; j<n-1; j++){
				//Initialisierung der inneren Elementen des lokalen Gitters
				xall[i*n+j] = init_val;
			}
		}
		//Initialisierung der ersten und der letzten Zeilen des lokalen Gitters
		for (int j=0; j<n; j++) {
			xall[j] = init_rand_val;
			xall[(n-1)*n+j] = init_rand_val;
		}
	}

	// Verteilung der Daten auf alle Prozesse
	// Ergänze hier: Berechnung der Lokalen Größe ! 
	//Achtung: Es ist kann sein, dass n%num_procs !=0 ist - daher nachdenekn! 

	int local_n = n;

	//allocate data and boundaries:
	if(num_p ==1 ){
		local_n = n-2;
		x = xall;
		xnew = (double*)calloc(n*n, sizeof(double)); 
	}
	else{
		x = (double*)calloc((local_n+2)*n, sizeof(double)); 
		xnew = (double*)calloc((local_n+2)*n, sizeof(double));
	}
	//Ergänzen: Verteilung der Daten von Prozess 0 an die anderen Prozesse! 

	//Tipp: Hier einfach einmal das Feld aufzeichnen um zu Verstehen, welcher Prozess was berechnen soll
	//und welche Daten er dafür braucht. Das Hilft sehr beim Verständnis! 

	do{
		itcnt++;
		diffnorm = one_jacobi_iteration(x,xnew,n,local_n+2);
		// Hier die Ränder Tauschen. Wenn wir nur einen Prozess haben, müssen auch die Ränder in der y-Richting getauscht werden! 
		if(num_p == 1)
			apply_boundary_single(x, xnew,n);
		else 
			apply_boundary(x, xnew,n, local_n+2);

		double *tmp = x;
		x= xnew;
		xnew = tmp;

		// Hier die globale Diffnorm berechnen! 

		diffnorm = sqrt(diffnorm);
		if(rank==0 && itcnt % 10 == 0)
			printf("Diffnorm %f \n", diffnorm);//Sie können die diffnorm verwenden, um zu testen, ob ihr ergebnis richtig ist! 
	}while(itcnt < MAX_ITER);

	MPI_Finalize();
	free(x);
	free(xnew);
	if(rank ==0  && num_p !=1)
		free(xall);
	return 0;



}

