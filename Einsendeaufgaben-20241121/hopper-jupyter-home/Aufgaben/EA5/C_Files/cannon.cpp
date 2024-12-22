#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <sys/time.h>
#include <stdlib.h>
#include <cstdio>
 #include <unistd.h>
//Abkürzung für die Überprüfung, ob der aktuelle Prozess der Wurzelprozess ist
#define IS_ROOT (rank == 0)	

#define PRINTSIZELIMIT 11
//#define DEBUG 


void init_matrix(double **A_out, int n, int m, bool zeroM){
    int m_size = n*m;
    const size_t bytes = m_size*sizeof(double); 
    *A_out = (double*)malloc(bytes);
    
    for(int i=0; i<m_size; i++){
        if(zeroM){
            (*A_out)[i]=0;
        }else{
            (*A_out)[i]=rand()%100;
        }
        
    }
};



/** Implementiert die sequenzielle Version der Matrix-Matrix Multiplikation 
* @param n die Anzahl Elemente pro Matrixblock. n muss eine Quadratzahl sein. 
* @param *a, *b Verweise auf die Eingabematrizen 
* @param *c Verweis auf die Ausgabematrizen**/
void MatrixMultiply(int n, double *a, double *b, double *c) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
                c[i * n + j] += a[i * n + k] * b[k * n + j];
}


bool isSquare(int x) {
	const float sqr = sqrtf(x);
	return (floor(sqr) == ceil(sqr));
}

/**
 * Implementiert Cannon Algorithmus für Multiplikation von zwei Matrizen.
 * @param n die Anzahl Elemente pro Matrixblock. n muss eine Quadratzahl sein.
 * @param *a, *b Verweise auf Blöcke der Eingabematrizen.
 * @param *c Verweise auf den Block der Ausgabematrix.
 * @param comm MPI_Communicator, der alle kommunizierenden Prozessoren beinhaltet.
 *
 * Bitte beachten Sie die Modifikation der initialen des Algorithmus (s. Grama et al.) 
 * Please observe the changes to the source code of Grama et al. for the initial matrix alignment and
 * the restoration of the original distribution. (The original code in the book leads to a deadlock.)
 */
void MatrixMatrixMultiply(int n, double *a, double *b, double *c, MPI_Comm comm) {
    int i;
    int nlocal;
    int npes, dims[2], periods[2];
    int myrank, my2drank, mycoords[2];
    int uprank, downrank, leftrank, rightrank;
    int shiftsource, shiftdest;
    MPI_Status status;
    //
    MPI_Comm comm_2d;

    /*Schritt 1: Sammlung der Informationen über den von MPI vorgegebenen Kommunikator 
      ================================================================================*/
    MPI_Comm_size(comm, &npes);
    MPI_Comm_rank(comm, &myrank);

    /*Schritt 2: Einrichtung einer Kartesischen Topologie 
      ===================================================*/
    dims[0] = dims[1] = sqrt(npes);
    periods[0] = periods[1] = 1;
    MPI_Cart_create(comm, 2, dims, periods, 1, &comm_2d);

    /* Bestimmung des Ranges und der Koordinaten bezüglich der neuen Topologie */
    MPI_Comm_rank(comm_2d, &my2drank);
    MPI_Cart_coords(comm_2d, my2drank, 2, mycoords);
    
     /*Schritt 3: Initiale Verschiebung
      =================================*/
    /* Berechnung der Ränge der Komminikationspartners links, rechts, oben und unten*/
    MPI_Cart_shift(comm_2d, 1, -1, &rightrank, &leftrank);	//i. e. dim[1] is X-coord
    MPI_Cart_shift(comm_2d, 0, -1, &downrank, &uprank);		//i. e. dim[0] is Y-coord

    /* Bestimmung der Größe von lokalen Matrizen*/
    nlocal = n / dims[0];

    #ifdef DEBUG
    if (1) {
        printf(
                "Process %d: my2drank: %d, "
                "mycoords[X]: %d, mycoords[Y]: %d, rightrank: %d, uprank: %d\n",
                my2drank, my2drank, mycoords[1], mycoords[0], rightrank, uprank);
    printf("Process mit Rang %d\n", myrank);
    printf("und Koordinaten %d, %d\n", mycoords[0], mycoords[1]);
    printf("Matrix A before alignment\n");
    print_matrix(a,nlocal,nlocal);
    printf("Matrix B before alignment\n");
    print_matrix(b,nlocal,nlocal);
    }
    #endif
    

    /* Durchführung der initialen Verschiebung für die Matrix A */
    MPI_Cart_shift(comm_2d, 1, -mycoords[0], &shiftsource, &shiftdest);
    //This is different to the solution from the book by GRAMA:
    //above we chose that dim[1] is X-coord (column)
    //Matrix A has to be shifted left according to the own Y-position
    #ifdef DEBUG
    if (1) {
    printf(
            "Process %d: Alignment for A  from %d --> %d; from %d --> %d;\n",
            myrank, my2drank, shiftdest, shiftsource, my2drank);
    }
    #endif

    MPI_Sendrecv_replace(a, nlocal * nlocal, MPI_DOUBLE, shiftdest, 1,
            shiftsource, 1, comm_2d, &status);

    /* Durchführung der initialen Verschiebung für die Matrix B */
    MPI_Cart_shift(comm_2d, 0, -mycoords[1], &shiftsource, &shiftdest);
    //This is different to the solution from the book by GRAMA:
    //above we chose that dim[0] is Y-coord (column)
    //Matrix B has to be shifted up according to the own X-position
    
    
    #ifdef DEBUG
    if (1) {
        printf("===========================================");
        printf("Process mit Rang %d\n", myrank);
        printf("und Koordinaten %d, %d\n", mycoords[0], mycoords[1]);
        printf("Matrix A after send and shift\n");
        print_matrix(a,nlocal,nlocal);
        printf("Matrix B after send and shift\n");
        print_matrix(b,nlocal,nlocal);

        printf(
                "Process %d: Alignment for B  from %d --> %d; from %d --> %d;\n",
                myrank, my2drank, shiftdest, shiftsource, my2drank);
        }
    #endif
    MPI_Sendrecv_replace(b, nlocal * nlocal, MPI_DOUBLE, shiftdest, 1,
            shiftsource, 1, comm_2d, &status);

    /* Hauptschleife */
    for (i = 0; i < dims[0]; i++) {
        
        /*Lokale Berechnungen
          -------------------*/
        MatrixMultiply(nlocal, a, b, c); /*c=c+a*b*/
        
        /*Kommunikation
          -------------*/
        /* Verschiebung der Matrix A um einen Block nach links */
        MPI_Sendrecv_replace(a, nlocal * nlocal, MPI_DOUBLE, leftrank, 1,
                rightrank, 1, comm_2d, &status);

        /* Verschiebung der Matrix B um einen Block nach oben */
        MPI_Sendrecv_replace(b, nlocal * nlocal, MPI_DOUBLE, uprank, 1,
                downrank, 1, comm_2d, &status);
         #ifdef DEBUG
        if (1) {
            printf("Process mit Rang %d\n", myrank);
            printf("und Koordinaten %d, %d\n", mycoords[0], mycoords[1]);
            printf("Matrix A\n");
            print_matrix(a,nlocal,nlocal);
            printf("Matrix B\n");
            print_matrix(b,nlocal,nlocal);
        }
        #endif
    }

    /* Herstellung der initialen Verteilung von Matrizen A und B */
    MPI_Cart_shift(comm_2d, 1, +mycoords[0], &shiftsource, &shiftdest);
    //again, this is different to the solution from the book by GRAMA:
    MPI_Sendrecv_replace(a, nlocal * nlocal, MPI_DOUBLE, shiftdest, 1,
                                       shiftsource, 1, comm_2d, &status);

    MPI_Cart_shift(comm_2d, 0, +mycoords[1], &shiftsource, &shiftdest);
    //again, this is different to the solution from the book by GRAMA:
    MPI_Sendrecv_replace(b, nlocal * nlocal, MPI_DOUBLE, shiftdest, 1,
            shiftsource, 1, comm_2d, &status);

    MPI_Comm_free(&comm_2d); 
}



// start_main
// >>> main start
int main(int argc, char *argv[]) {
// <<< main start

int size, rank;
//Verweise auf die Ein- und Ausgabematrizen (allokiert beim Wurzelprozess)
double *matA, *matB; 
double *matC;
double *subMatA, *subMatB, *subMatC; 

int n = 576; //Teilbar durch 2,4, 9 und 16

MPI_Init(NULL, NULL);
MPI_Comm_size(MPI_COMM_WORLD, &size);
if (!isSquare(size)){
    printf("Fehlermeldung: Das Program soll mit eine quadratische Anzahl Prozessoren gestartet werden!\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
}
// start_full
//Anzahl Matrixblöcke in die X-Richtung und die Y-Richtung
int tilesX; 
int tilesY;

//Anzahl Elemente pro Block in die X-Richtung
int tileSizeX; 	
int tileSizeY;

//Bestimme die Anzahl der Matrixblöcke
tilesX=sqrtf(size);	
tilesY = tilesX;

//Bestimme die Größe eines Matrixblocks
tileSizeX = n/tilesX; 
//TODO check if we really need it
tileSizeY = n/tilesY;
// start_init
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
if(IS_ROOT){ 
    //Initialisierung
    
    init_matrix(&matA, n, n, false);
    init_matrix(&matB, n, n, false);
    init_matrix(&matC, n, n, true);
    
}

if(size > 1) 
{
//Allokierung der lokalen Matrizen 
   if(!(subMatA = (double*) malloc(tileSizeX*tileSizeX*sizeof(double)))){
       fprintf(stderr,"Prozessor %d: Speicherzuweisung für %d*%d doubles in Matrix subA ist fehlgeschlagen.\n", 
            rank, tileSizeX, tileSizeX);
    //Freigabe der bis jetzt allokierten Speicher
       if(IS_ROOT) {free(matA); free(matB), free(matC);}
       MPI_Abort(MPI_COMM_WORLD, 4);
  }
   if(!(subMatB = (double*) malloc(tileSizeX*tileSizeX*sizeof(double)))){
       fprintf(stderr,"Prozessor %d: Speicherzuweisung für %d*%d doubles in Matrix subB ist fehlgeschlagen.\n", rank, tileSizeX, tileSizeX);
      //Freigabe der bis jetzt allokierten Speicher
       if(IS_ROOT) {free(matA); free(matB), free(matC);}
       free(subMatA);
       MPI_Abort(MPI_COMM_WORLD, 5);
       MPI_Finalize();
       exit(5);
    }
    if(!(subMatC = (double*) malloc(tileSizeX*tileSizeX*sizeof(double)))){
        fprintf(stderr,"Prozessor %d: Speicherzuweisung für %d*%d doubles in Matrix subC ist fehlgeschlagen.\n", rank, tileSizeX, tileSizeX);
        //Freigabe der bis jetzt allokierten Speicher
        if(IS_ROOT) {free(matA); free(matB), free(matC);}
        free(subMatA);
        free(subMatB);
        MPI_Abort(MPI_COMM_WORLD, 6);
        MPI_Finalize();
        exit(6);
    }


//Verteilung der Matrix A 
    MPI_Scatter(matA, tileSizeX*tileSizeX, MPI_DOUBLE,
           subMatA, tileSizeX*tileSizeX, MPI_DOUBLE,
           0, MPI_COMM_WORLD);
//Verteilung der Matrix B
    MPI_Scatter(matB, tileSizeX*tileSizeX, MPI_DOUBLE,
           subMatB, tileSizeX*tileSizeX, MPI_DOUBLE,
           0, MPI_COMM_WORLD);

}
// end_init
// start_compute

if(size == 1) {
    MatrixMultiply(n, matA,matB, matC);
}
else {
    MatrixMatrixMultiply(tilesX*tileSizeX, subMatA, subMatB, subMatC, MPI_COMM_WORLD);

//Sammlung der Ergebnisse aller Prozessoren
    MPI_Gather(subMatC, tileSizeX*tileSizeX, MPI_DOUBLE,
        matC, tileSizeX*tileSizeX, MPI_DOUBLE,
        0, MPI_COMM_WORLD);
}
// end_compute
// end_full 
if(IS_ROOT){

    free(matA);
    free(matB);
    free(matC);
}
if(size>1) {
  free(subMatA);
  free(subMatB);
  free(subMatC);
}


MPI_Finalize();
// end_main
// >>> main end
return 0;
}
// <<< main end

// MODIFICATION MAIN DONE
