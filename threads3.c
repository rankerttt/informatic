#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define N 1000

struct timespec t0[N], t1[N];

void * any_func (void * arg)
{
  while (1)
  {
    fprintf (stderr, ".");
    sleep (10);
  }
  return NULL;
}

int main (void)
{
  pthread_t threads[N];
  void * result = NULL;
  double dt = 0;

  for (int i = 0; i < N; i++)
  {
    if (pthread_create (&threads[i], NULL, &any_func, NULL) != 0)
    {
      fprintf (stderr, "Error\n");
      return 1;
    }
  }

  sleep (5);

  for (int k = 0; k < N; k++)
  {
    clock_gettime (CLOCK_MONOTONIC, &t0[k]);
    pthread_cancel (threads[k]);
  }

  for (int k = 0; k < N; k++)
  {
    pthread_join (threads[k], &result);
    clock_gettime (CLOCK_MONOTONIC, &t1[k]);

    if (result == PTHREAD_CANCELED)
    {
      dt += (t1[k].tv_sec - t0[k].tv_sec) + (t1[k].tv_nsec - t0[k].tv_nsec) / 1e9;
    }
    else
    {
      fprintf (stderr, "Thread %d was not canceled\n", k);
    }
  }

  printf ("total: %f s, average: %f s\n", dt, dt / N);
  return 0;
}
