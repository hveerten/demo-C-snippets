////////////////////////////////////////////////////////////////////////////////
// testing multi-D arrays with openacc
//
// compile with pgcc -acc=gpu -Wall openacc-multi-D-array-pointer.c
//
// This snippet demonstrates that a multi-dimensional array allocated as a
// contiguous block is properly duplicated onto device memory when using the
// standard 'copyin' and 'create' syntax. The added complexity relative to
// openacc-multi-D-array.c is that the arrays are now members of a struct
// while we access the struct via a pointer
//
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#define RES 10

typedef struct grid
{
  double **A;
  double **B;
} grid;

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

void copyAB(grid *p_g)
{
  int i, j;
  
  // while on the device, copy content from A into B
  #pragma acc parallel loop private (j) present(p_g) collapse(2)
  for (i = 0; i < RES; i++)
    for (j = 0; j < RES; j++)
      p_g->B[i][j] = p_g->A[i][j];
}

////////////////////////////////////////////////////////////////////////////////

int main()
{
  grid g;
  grid* p_g = &g;
  int i, j;
  
  p_g->A = allocate_2D_array_double(RES, RES);
  p_g->B = allocate_2D_array_double(RES, RES);
  
  // initialize values for array A on the host
  for (i = 0; i < RES; i++)
    for (j = 0; j < RES; j++)
      p_g->A[i][j] = (double) (i * RES + j);
  
  // copy the content of A onto the device, while merely replicating the
  // structure of B on the device
  #pragma acc enter data copyin(p_g[0:1])
  #pragma acc enter data copyin(p_g->A[0:RES][0:RES])
  #pragma acc enter data create(p_g->B[0:RES][0:RES])

  copyAB(p_g);
  
  // copy the device version B content onto the host. Without this line, the
  // host values of B should remain uninitialized
  #pragma acc update self(p_g->B[0:RES][0:RES])

  // show the content of host version B on the screen
  for (i = 0; i < RES; i++)
    for (j = 0; j < RES; j++)
      printf("B[%d][%d] = %lf\n", i, j, p_g->B[i][j]);
  
  // deallocate memory on device and host
  #pragma acc exit data delete(p_g->A)
  #pragma acc exit data delete(p_g->B)
  #pragma acc exit data delete(p_g)
  
  free_2D_array_double(p_g->A);
  free_2D_array_double(p_g->B);
  
  return 0;
}
