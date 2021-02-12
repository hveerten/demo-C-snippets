// testing classes & static members
//
// motivation: to be able to set up a class of which many instances can exist,
// for example as private variables in a parallel for loop, while still having
// all the instances draw from the same array in memory.
//
// compiled with pgc++ class-static-member-openacc.cpp -Wall -acc=gpu


#include <stdio.h>

// define a class for testing, including a static member array
class c_test
{
  public:
   
    // declare some static class members that will exist outside of any instance
    // of the class (ie they're basically global variables and functions)
    static void initialize_dataset(int res_arg);
    static void deallocate_statics();

    #pragma acc routine seq
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
#pragma acc declare create(c_test::dataset)
int c_test::res = 0;
#pragma acc declare create(c_test::res)

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
  #pragma acc enter data create(dataset[0:res])

  printf("initializing data set...\n");
  for (i = 0; i < res; i++)
  {
    dataset[i] = i;
  }
  
  #pragma acc update device(dataset[0:res])
}

void c_test::deallocate_statics()
{
  if (dataset != NULL) 
  { 
    #pragma acc exit data delete(dataset)
    delete[] dataset;
    dataset = NULL;
  }
}

int main()
{ 
  int res = 100;
  c_test::initialize_dataset(res);

  //c_test C_static_acc; // create a local instance of the class
 
  // now that we have the static member all set up, we need to get this
  // information onto the GPU memory (ie the 'device')
 
  int i;
  long* dataset2 = new long[res];
  for (i = 0; i < res; i++) dataset2[i] = 0;
  
  #pragma acc enter data copyin(dataset2[0:res])
  
  #pragma acc parallel loop 
  for (i = 0; i < res; i++)
  {
    c_test C; // create a local instance of the class
    dataset2[i] = C.report_number(i);
  }

  // update the host memory with what was computed on the device
  #pragma acc update self(dataset2[0:res])
  
  for (i = 0; i < res; i++)
    printf("entry %i has value %li\n", i, dataset2[i]);

  // wrap up the GPU memory version of variables that were copied by hand
  //#pragma acc exit data delete(C_static_acc)
  
  // release the relevant memory
  delete[] dataset2;
  c_test::deallocate_statics();
  return 0;
}
