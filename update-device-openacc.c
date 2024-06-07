////////////////////////////////////////////////////////////////////////////////
//
// contiguous arrays multi-D versus flat
//
// Demonstrating the syntax of update device statement
//
// compile with pgcc -lm -acc
//
////////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////

int main()
{
  double *A;
  
  A = malloc(10 * sizeof(double));
  
  for (int i = 0; i < 10; i++)
    A[i] = 1.;
  
  for (int i = 0; i < 10; i++)
  {
    printf("host: A[%d] = %e\n", i, A[i]);
  }
  
#pragma acc enter data copyin(A[0:10])

#pragma acc parallel
  {
    for (int i = 0; i < 10; i++)
    {
      printf("device: A[%d] = %e\n", i, A[i]);
    }
  }  

  // now change values on the host, but only copy part of the changes onto 
  // device in a manner that demonstrates the syntax of [A:B] notation, where
  // B is the *number of entries* rather than the *final entry plus one*
  
  for (int i = 0; i < 10; i++)
    A[i] = 2.;

#pragma acc update device(A[2:3])  

#pragma acc parallel
  {
    for (int i = 0; i < 10; i++)
    {
      printf("device: A[%d] = %e\n", i, A[i]);
    }
  }  

  #pragma acc exit data delete(A)
  free(A);
}

