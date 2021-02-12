// testing classes & static members
//
// motivation: to be able to set up a class of which many instances can exist,
// for example as private variables in a parallel for loop, while still having
// all the instances draw from the same array in memory.
//


#include <stdio.h>

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
  printf("static member pointing to %p\n", c_test::dataset);
  
  // calling a static member function does not require any instances of the
  // class to exist.
  c_test::initialize_dataset(1000);

  printf("static member pointing to %p\n", c_test::dataset);

  // generating an instance of the class, in order to use the 
  // report_number function
  c_test C;
  
  printf("entry number 492 is equal to %li\n", C.report_number(492));
  
  // deallocation of static class members has to happen manually, and is not
  // covered by a class destructor (which is bound to instances of the class)
  c_test::deallocate_statics();

  printf("static member pointing to %p\n", c_test::dataset);
  return 0;
}
