#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define IS_ROOT (rank == 0)


int main(int argc, char *argv[]) {
	int max=1<<22;
	MPI_Init(NULL, NULL);
	int rank, size = 0;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	char* sendbuf = (char*) malloc(max);
	char* recvbuf = (char*) malloc(max);
	double t1,t2;

	free(sendbuf);
	free(recvbuf);
	MPI_Finalize();

	return 0;
}
