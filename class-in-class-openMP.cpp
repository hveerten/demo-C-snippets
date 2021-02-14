// testing whether classes within classes also get copied with firstprivate
//
// HJvE, Feb 14, 2021
//
//
// compiled with g++ class-in-class-openMP.cpp -fopenmp -Wall

#include <stdio.h>
#include <omp.h>

// define a class for testing, including a member array
class c_test_inner
{
  public:
   
    int n;
};

class c_test_outer
{
  public:
    c_test_inner Cin;
};

int main()
{
  c_test_outer C;
  int res = 10;

  C.Cin.n = 42;

  #pragma omp parallel for firstprivate(C)
  for (int i=0; i < res; i++)
  {
    int my_ID = omp_get_thread_num();
    printf("i = %i, my_ID = %i, C.Cin.n = %i\n", i, my_ID, C.Cin.n); fflush(stdout);
  }

  #pragma omp parallel for private(C)
  for (int i=0; i < res; i++)
  {
    int my_ID = omp_get_thread_num();
    printf("i = %i, my_ID = %i, C.Cin.n = %i\n", i, my_ID, C.Cin.n); fflush(stdout);
  }

  return 0;
}
