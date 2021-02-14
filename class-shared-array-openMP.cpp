// testing classes and dynamic array members
//
// HJvE, Feb 14, 2021
//
// This demonstrates how to work in openMP with local copies of a class
// within a parallel region, while still having these local copies share
// a connection to a single memory location. It is in principle straightforward,
// because the local copies just start out with copies of the pointer
// initialized to the same part of memory. The pitfall is that the local copies
// get discarded at the end of the parallel region, at which point the
// destructor of the class gets called. Be careful therefore not to put 
// a memory deallocation (ie, free or delete[]) in the destructor.
//
// compiled with g++ class-shared-array-openMP.cpp -fopenmp -Wall

#include <stdio.h>
#include <omp.h>

// define a class for testing, including a member array
class c_test
{
  public:
   
    long* dataset;
    int res;
    
    c_test();

    void initialize_dataset(int a_res);
    void deallocate(); // cannot be done in a destructor, if the memory
      // allocation is to survive the parallel region.

  protected:
};

c_test :: c_test()
{
  dataset = NULL; // this initialization is fine, because the default value
    // may still be overridden by a 'firstprivate' statement.
}

void c_test :: deallocate()
{
  // don't do this in a destructor, because it will be called multiple times
  // if a destructor is triggered by a threadprivate copy of the class
  if (dataset != NULL)
  {
    delete[] dataset;
  }
}

void c_test::initialize_dataset(int a_res)
{
  long i;
  
  res = a_res;
  dataset = new long[res];

  for (i = 0; i < res; i++)
  {
    dataset[i] = i;
  }
}

int main()
{
  c_test C;
  int res = 10;
  long* dataset2 = new long[res];
  
  // calling a static member function does not require any instances of the
  // class to exist.
  C.initialize_dataset(res);

  #pragma omp parallel for firstprivate(C)
  for (int i=0; i < res; i++)
  {
    int my_ID = omp_get_thread_num();
    printf("i = %i, my_ID = %i, dataset pointer = %p\n", i, my_ID, &C.dataset[i]); fflush(stdout);
    dataset2[i] = C.dataset[i];
    C.dataset[i] = my_ID;
  }

  for (int i=0; i < res; i++)
  {
    printf("dataset2: %i, %li\n", i, dataset2[i]);
  }

  for (int i=0; i < res; i++)
  {
    printf("dataset: %i, %li\n", i, C.dataset[i]);
  }

  C.deallocate();
  delete[] dataset2;
  return 0;
}
