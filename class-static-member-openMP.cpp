// testing classes & static members
//
// motivation: to be able to set up a class of which many instances can exist,
// for example as private variables in a parallel for loop, while still having
// all the instances draw from the same array in memory.
//
// compiled with g++ class-static-member-openMP.cpp -fopenmp -Wall


#include <stdio.h>
#include <omp.h>

// define a class for testing, including a static member array
class c_test
{
  public:
   
    // declare some static class members that will exist outside of any instance
    // of the class (ie they're basically global variables and functions)
    static void initialize_dataset(int res_arg);
    static void deallocate_statics();

    long report_number(long i); // a normal class member, requiring an instance
      // of the class

    static long* dataset; // left as 'public', to be able to print its value
      // directly by accessing the variable from main.

  protected:
  
    static int res; // like 'dataset' this is an example of a declaration of a
      // static class member variable. Any such variable declaration (including
      // 'dataset' above) is of 'incomplete type'.
};

// If the static functions are to modify the static variables, these static
// variables must be defined in order to become a 'complete type'. 
long* c_test::dataset = NULL;
int c_test::res = 0;

// functions, too, must be defined in addition to being declared, but this
// is general to all functions, class/struct members or not.
long c_test::report_number(long i)
{
  return dataset[i];
}

void c_test::initialize_dataset(int res_arg)
{
  long i;
  
  // set the resolution to the provided resolution. Note that this line would
  // lead to a compile error if the variable res had not been made of
  // complete type already in the global scope.
  res = res_arg;
  dataset = new long[res];

  printf("initializing data set...\n");
  for (i = 0; i < res; i++)
  {
    dataset[i] = i;
  }
}

void c_test::deallocate_statics()
{
  if (dataset != NULL) { delete[] dataset; dataset = NULL; }
}

int main()
{ 
  // start by making sure that the test is indeed run in parallel

  #pragma omp parallel
  {
    int my_ID = omp_get_thread_num();
    int ID_max = omp_get_num_threads(); // this will be equal to max_threads
      // normally, but equal to one if -fopenmp not used (and the pragma
      // command is ignored)
    
    if (my_ID == 0) printf("ID_max (should be >1): %i\n", ID_max);
  }
      
  int res = 100;
  
  c_test::initialize_dataset(res);
 
  
  int i;
  long* dataset2 = new long[res];
  
  // assign values to dataset2 in a parallel for loop. This one will lead to
  // a clear 'false sharing' issue given that different threads will be
  // assigning values to adjacent spots in memory, thereby getting in the way
  // of each other's caches. That is however not the point of this loop. It is
  // to demonstrate that we can easily generate tons of instances in parallel
  // without the static class member dataset becoming an issue. There will be
  // no false sharing issue btw when reading from adjacent memory addresses.
  #pragma omp parallel for private (i)
  for (i = 0; i < res; i++)
  {
    c_test C; // create a local instance of the class
    dataset2[i] = C.report_number(i);
  }
  
  for (i = 0; i < res; i++)
    printf("entry %i has value %li\n", i, dataset2[i]);
  
  delete[] dataset2;
  c_test::deallocate_statics();
  return 0;
}
