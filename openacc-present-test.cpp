#include <stdlib.h>
#include <stdio.h>

typedef struct grid
{
  int N;
  double *X;
} grid;

void allocate(grid* g, int N)
{
  g->N = N;
  g->X = (double*) malloc(sizeof(double) * g->N);

  #pragma acc enter data copyin(g[0:1])
  #pragma acc enter data create(g->X[0:N])
}

void release(grid* g)
{
  #pragma acc exit data delete(g->X)
  #pragma acc exit data delete(g)

  free(g->X);
}

void fill(grid * g)
{
  int i;

  #pragma acc parallel loop present(g)
  for (i = 0; i < g->N; i++)
  {
    g->X[i] = 42; 
  }
  #pragma acc update self(g->X[:g->N])
  for (i = 0; i < 4; i++)
  {
    printf("%d : %f \n",i,g->X[i]);
  }
}

int main()
{
  grid g;

  allocate(&g, 10);

  fill(&g);

  release(&g);

  return 0;
}

