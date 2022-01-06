////////////////////////////////////////////////////////////////////////////////
// testing multi-D arrays with openacc
//
// compile with pgcc -acc=gpu openacc-multi-D-array.c
//
// This snippet demonstrates that a multi-dimensional array allocated as a
// contiguous block is properly duplicated onto device memory when using the
// standard 'copyin' and 'create' syntax.
//
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#define RES 10


double** allocate_2D_array_double(const int N_x, const int N_y)
{
  double **A;
  int i_x;

  A = malloc(N_x * sizeof(double*));
  A[0] = malloc(N_x * N_y * sizeof(double));
  for (i_x = 0; i_x < N_x; i_x++)
  {
    A[i_x] = A[0] + i_x * N_y;
  }
  
  return A;
}

////////////////////////////////////////////////////////////////////////////////

void free_2D_array_double(double **A)
{
  free(A[0]);
  free(A);
}

////////////////////////////////////////////////////////////////////////////////

int main()
{
  // The main program will declare 2 arrays, and copy content from one onto the
  // other on the device.
  
  double** A;
  double** B;
  int i, j;
  
  A = allocate_2D_array_double(RES, RES);
  B = allocate_2D_array_double(RES, RES);
  
  // initialize values for array A on the host
  for (i = 0; i < RES; i++)
    for (j = 0; j < RES; j++)
      A[i][j] = (double) (i * RES + j);
  
  // copy the content of A onto the device, while merely replicating the
  // structure of B on the device
  #pragma acc enter data copyin(A[0:RES][0:RES])
  #pragma acc enter data create(B[0:RES][0:RES])

  // while on the device, copy content from A into B
  #pragma acc parallel loop private (j) present(A, B) collapse(2)
  for (i = 0; i < RES; i++)
    for (j = 0; j < RES; j++)
      B[i][j] = A[i][j];
  
  // copy the device version B content onto the host. Without this line, the
  // host values of B should remain uninitialized
  #pragma acc update self(B[0:RES][0:RES])

  // show the content of host version B on the screen
  for (i = 0; i < RES; i++)
    for (j = 0; j < RES; j++)
      printf("B[%d][%d] = %lf\n", i, j, B[i][j]);
  
  // deallocate memory on device and host
  #pragma acc exit data delete(A)
  #pragma acc exit data delete(B)
  
  free_2D_array_double(A);
  free_2D_array_double(B);
  
  return 0;
}
