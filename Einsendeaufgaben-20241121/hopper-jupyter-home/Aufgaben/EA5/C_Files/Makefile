CC := clang++ -fopenmp
MPICC := mpic++
CFLAGS = -O3 -lm  
LIBS = -lm

all: Laplace_MPI Laplace_OMP SPMV_MPI SPMV_OMP cannon 

cannon: cannon.cpp:
	$(MPICC) $(CFLAGS) $< $(LIBS) -o $@

Laplace_OMP: Laplace_OMP.cpp
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

SPMV_OMP: SPMV_OMP.cpp
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

Laplace_MPI: Laplace_MPI.cpp
	$(MPICC) $(CFLAGS) $< $(LIBS) -o $@

SPMV_MPI: SPMV_MPI.cpp
	$(MPICC) $(CFLAGS) $< $(LIBS) -o $@

clean:
	rm -f Laplace_OMP SPMV_OMP Laplace_MPI SPMV_MPI
