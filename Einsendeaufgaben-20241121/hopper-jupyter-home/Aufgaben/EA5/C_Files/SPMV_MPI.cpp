#include <iostream>
#include "csr_formatter.h" 
#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <functional>
#include <string> 
#include <mpi.h>



double randfrom(double min, double max) 
{
    double range = (max - min); 
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

double* init_vector(int size) {
   double* b = (double*)malloc(size*sizeof(double));
   for(int i=0; i<size; i++) {
       b[i]=randfrom(-1, 1);   
   }
     return b;
 }

void SpMV_MPI(double *data, int *cols, int *rowptr, int n_local, double *b, double *c){

}

//Für den Vergleich die Klassiche Berechnung
void DMV(double *data, int n, int k, double *b, double *c){
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


int main(int argc, char *argv[]) {


#define ABS(a) ((a) >0 ? (a) :-(a))
	int my_id, num_p;
	double *b;
	double *c ;
	CSR* Mat;

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_p);
	int rank = my_id;
	int n = 0;
	//liest die Matrix   ein dabei liest jeder Prozess nur seinen Teil der Daten! Liest die Daten so ein, dass die nicht-Null Elemente gleich verteilt sind! Bitte Anpassen für den 2. Teil der Aufgabenstellung! 
	Mat = assemble_part_csr_matrix( choose_file(n),my_id,num_p );

	if(my_id==0) 
		b=init_vector(Mat->M);
	else 
		b= (double*)malloc(sizeof(double)*Mat->M);
	//Verteile den Vektor b an alle Prozesse: 

	MPI_Bcast(b, Mat->M, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	// localer Teil der Lösung 
	double* my_c =(double*)malloc((Mat->local_n+1)*sizeof(double));
	//hier bitte den Rest Implementieren!  
	MPI_Barrier(MPI_COMM_WORLD);

	SpMV_MPI(Mat->val, Mat->col_ind, Mat->row_ptr, Mat->local_n, b, my_c);
	
	
	//Sammeln der Ergebnisse Implementieren 
	if(my_id ==0) {
		c =(double*)malloc(Mat->M*sizeof(double));
	}
	
	free_CSR(Mat);
	//Test for small Matrix
	if(Mat->N < (1<<15)&& my_id == 0) {
		double *m = assemble_dense_matrix(choose_file(n));
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

	free(Mat);
	free(my_c);
	if(my_id == 0)
	    free(c);
	MPI_Finalize();


	return 0;
}

