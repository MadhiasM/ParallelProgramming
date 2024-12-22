#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <math.h> 
#include <string.h>
#define min(a,b) a>b ? b : a
using namespace std;

struct CSR {
	double* val;
	int*	col_ind;
	int* row_ptr;
    int M;
    int N;
    int L;
    int local_n; 
    int local_l;
    int first_row;
    int last_row;
};

void free_CSR(CSR *mat){

    free(mat->val);
    free(mat->row_ptr);
    free(mat->col_ind);
}

#define ALLOC_SIZE 1024
CSR* assemple_part_csr_matrix2(std::string filePath, int rank, int num_procs){
    
	CSR* matrix=(CSR*)malloc(sizeof(CSR));
    
	std::ifstream fin(filePath);
    int p=0,q=0;
    int last_row = -1;
	// Ignore headers and comments:
	while (fin.peek() == '%') fin.ignore(2048, '\n');
	// Read defining parameters:
	fin >> matrix->M >> matrix->N >> matrix->L;
    /*get local number of values */
    matrix->local_n = (matrix->N)/num_procs;
    int rest = matrix->N%num_procs;
     if(rest >rank) matrix->local_n+=1;
    /* now get the start point */
    int my_start = rank*(matrix->N/num_procs);
    my_start+=min(rank,rest);

    matrix->local_l=0;
    matrix->first_row=my_start;
    
    last_row = my_start-1;
    
    matrix->val=(double*) malloc(sizeof(double)*matrix->L/num_procs*2);
    matrix->col_ind=(int*) malloc(sizeof(int)*matrix->L/num_procs*2);
    // Here at least we know the size... 
    matrix->row_ptr = (int*) malloc(sizeof(int)*(matrix->local_n+5));
    int curr_size = ((int)((double)(matrix->L/num_procs*1.5))/ALLOC_SIZE)*ALLOC_SIZE;
    int local = 0;
	for (int l = 0; l < matrix->L; l++){
		int row, col;
		double data;
		fin >> col >> row >> data;
        row = row-1;
       if (row< matrix->first_row ) continue;
       else if (row > matrix->first_row+matrix->local_n) { break;}
        matrix->col_ind[local]=(col-1);
		matrix->val[local]=data;
	   if (row > last_row){
           for(int j=0; j<row-last_row; j++){
                matrix->row_ptr[p]=q;
                p++;
           }
           	last_row = row;
		}
        q++;
        local++;
        if(local > curr_size){
         curr_size+=ALLOC_SIZE;
         matrix->val=(double*) realloc(   matrix->val, sizeof(double)*curr_size);
         matrix->col_ind=(int*) realloc( matrix->col_ind,sizeof(int)*curr_size);

        }
        
           
	}
	matrix->row_ptr[p]=q;
    matrix->local_l=local;
    matrix->last_row=last_row;    

	fin.close();

    return matrix;
}

CSR* assemble_part_csr_matrix(std::string filePath, int rank, int num_procs){
    
	CSR* matrix=(CSR*)malloc(sizeof(CSR));
    
	std::ifstream fin(filePath);
    int p,q;
    int last_row = 0;
    p=0; q=0;
	// Ignore headers and comments:
	while (fin.peek() == '%') fin.ignore(2048, '\n');
	// Read defining parameters:
	fin >> matrix->M >> matrix->N >> matrix->L;
	//std::cout<<"M "<<matrix->M<<" N "<<matrix->N<<" L "<<matrix->L<<std::endl;
    /*get local number of values */
    matrix->local_l = matrix->L/num_procs;
    int rest = matrix->L%num_procs;
    if(rest >rank) matrix->local_l+=1;
    /* now get the start point */
    int my_start = rank*(matrix->L/num_procs);
    my_start+=min(rank,rest);

    
	//matrix->row_ptr = (int*) malloc(sizeof(int)*(matrix->M+1));
    matrix->val=(double*) malloc(sizeof(double)*matrix->local_l);
    matrix->col_ind=(int*) malloc(sizeof(int)*matrix->local_l);
    matrix->row_ptr = (int*) malloc(sizeof(int)*(matrix->local_l));

    int local = 0;
	for (int l = 0; l < matrix->L; l++){
		int row, col;
		double data;
		fin >> col >> row >> data;
         row = row-1;
       if (l<my_start ) { continue;}
       else if (l==my_start){ matrix->first_row=row; last_row=row-1;q=0; p=0;}
     
		matrix->col_ind[local]=(col-1);
		matrix->val[local]=data;
	  		if (row > last_row){
           for(int j=0; j<row-last_row; j++){
               matrix->row_ptr[p]=q;
                p++;
           }
		last_row=row;
		}	
        q++;
        local++;
        if (local == matrix->local_l){    matrix->last_row= row; break;}
           
	}


	matrix->row_ptr[p]=q;
    matrix->local_n=matrix->last_row-matrix->first_row+1;

    matrix->row_ptr= (int*) realloc(matrix->row_ptr, (p+1)*sizeof(int));
	fin.close();

    return matrix;
}


CSR* assemble_csr_matrix(std::string filePath){
    
	CSR* matrix=(CSR*)malloc(sizeof(CSR));

	std::ifstream fin(filePath);
    int p,q;
    p=0; q=0;
	// Ignore headers and comments:
	while (fin.peek() == '%') fin.ignore(2048, '\n');
	// Read defining parameters:
	fin >> matrix->M >> matrix->N >> matrix->L;
    //std::cout<<matrix->M << " "<<matrix->N<<" "<<matrix->L<<std::endl;
	int last_row = -1;
	matrix->row_ptr = (int*) malloc(sizeof(int)*(matrix->M+1));
    matrix->val=(double*) malloc(sizeof(double)*matrix->L);
    matrix->col_ind=(int*) malloc(sizeof(int)*matrix->L);
	for (int l = 0; l < matrix->L; l++){
		int row, col;
		double data;
		fin >> col >> row >> data;
		matrix->col_ind[l]=(col-1);
		matrix->val[l]=data;
        row = row-1;
		if (row > last_row){
			matrix->row_ptr[p]=q;
           for(int j=0; j<row-last_row; j++){
                matrix->row_ptr[p]=q;
                p++;
           }
		last_row=row;
		}	
        q++;
           
	}
    matrix->local_n=p;
	matrix->row_ptr[p]=q;
	fin.close();
    return matrix;
}

double* assemble_dense_matrix(std::string filePath){
    
	double* matrix; 
    int L,M,N;
    
    std::ifstream fin(filePath);
        while (fin.peek() == '%') fin.ignore(2048, '\n');

    fin >> M >> N >>L;    
    
   matrix =(double*) malloc(sizeof(double)*M*N);
   memset(matrix,0,M*N*sizeof(double));
	
    int p,q;
    p=0; q=0;
	// Ignore headers and comments:
	
	// Read defining parameters:

	for (int l = 0; l <L; l++){
		int row, col;
		double data;
		fin >> col >> row >> data;
		matrix[(col-1)+(row-1)*(N)] = data;    
	}

	fin.close();
    return matrix;
}

void store_matrix(CSR *Mat, std::string filename){
      ofstream MatFile;
      MatFile.open (filename);
      MatFile<<Mat->N<<" "<<Mat->M<<" "<<Mat->L<<std::endl;
      for(int i = 0; i<Mat->L; i++) {
       MatFile<<Mat->val[i]<<" ";
      }
     MatFile<<std::endl;
     for(int i = 0; i<Mat->L; i++) {
       MatFile<<Mat->col_ind[i]<<" ";
      }
    
     MatFile<<std::endl;
    MatFile<<Mat->local_n<<std::endl;
    for(int i = 0; i<Mat->local_n+1; i++) {
       MatFile<<Mat->row_ptr[i]<<" ";
      } 
         MatFile<<std::endl;
    MatFile.close();
    
}

CSR *read_mat(std::string name) {
  CSR* matrix=(CSR*)malloc(sizeof(CSR));
  std::ifstream fin(name);
  fin >> matrix->M >> matrix->N >> matrix->L;
  matrix->val= (double*)malloc(sizeof(double)*matrix->L);
  matrix->col_ind= (int*)malloc(sizeof(int)*matrix->L);
  for(int i=0; i<matrix->L; i++) {
      fin >> matrix->val[i];
  }
    for(int i=0; i<matrix->L; i++) {
      fin >> matrix->col_ind[i];
  }
  fin >> matrix->local_n;
    
  matrix->row_ptr = (int*) malloc(sizeof(int)*matrix->local_n+1);
  for(int i=0; i<matrix->local_n+1; i++) {
      fin >>matrix->row_ptr[i];
  }
    return matrix;
}
double* generate_dense_matrix(CSR *Mat){
  
    double *matrix = (double*) malloc(Mat->N*Mat->M*sizeof(double));
    int v =0;
    int start =0;
    for (int i=0; i<Mat->M; i++){
       int nonzero = Mat->row_ptr[i];
       int start = Mat->col_ind[nonzero];
        for(int k=0; k<Mat->N; k++){
            if(k==start){
                matrix[i*Mat->N+k]=Mat->val[v];
                v++;
                nonzero++;
                if(nonzero < Mat->row_ptr[i+1]) 
                start = Mat->col_ind[nonzero] ;
                 else start = -1;
            }
            else matrix[i*Mat->N+k]=0.0000;
        }
       // std::cout<<"nonzero 2 "<<nonzero<<" v "<<v<<" i "<<i<<std::endl;
    }
    
    return matrix;
}