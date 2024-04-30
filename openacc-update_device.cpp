// compile with pgc++ -acc
#include <stdio.h>
#include <stdlib.h>

#define RES 10

class test_struct
{
  public:
  static int N;
};

int test_struct::N;
// when the line below is uncommented, we need to manually start updating T.N
// Otherwise, I assume the host is just copied onto the device at the start of
// a parallel loop
#pragma acc declare create(test_struct::N)

int main()

{
  test_struct T;
  T.N = 3;
  
  
  int* A;
  A = new int[RES];
  #pragma acc enter data create(A[0:RES])
  
  // line below needed once N declared
  #pragma acc update device(T.N)
  
  #pragma acc parallel loop
  for (int i = 0; i < RES; i++)
  {
    A[i] = T.N * i;
  }
  
  #pragma acc update host(A[0:RES])
  
  for (int i = 0; i < RES; i++)
  {
    printf("%d, %d\n", i, A[i]);
  }
  
  delete[] A;
  #pragma acc exit data delete(A)
  
  return 0;
}
