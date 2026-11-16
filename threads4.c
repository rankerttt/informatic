#include <stdio.h>
#include <pthread.h>
#include <math.h>

#define N 20
#define T 40

struct data
{
  int b;
  double x;
  int first, last;
  double sum;
};

double summ[T];

double factorial (int n)
{
  double result = 1;
  for (int i = 1; i <= n; i++)
    result *= i;
  return result;
}

void * part_sin (void * arg)
{
  struct data * d = (struct data *) arg;

  d->sum = 0;
  for (int n = d->first; n < d->last; n++)
  {
    int k = 2 * n + 1;
    d->sum += pow (-1, n) * pow (d->x, k) / factorial (k);
  }

  summ[d->b] = d->sum;
  return NULL;
}

int main (void)
{
  pthread_t threads[T];
  struct data d[T];
  double x = 1.0;
  scanf ("%lf", &x);

  for (int i = 0; i < T; i++)
  {
    d[i].b = i;
    d[i].x = x;
    d[i].first = i * N / T;
    d[i].last = (i + 1) * N / T;

    if (pthread_create (&threads[i], NULL, &part_sin, &d[i]) != 0)
    {
      fprintf (stderr, "Error\n");
      return 1;
    }
  }

  for (int i = 0; i < T; i++)
    pthread_join (threads[i], NULL);

  double total = 0;
  for (int i = 0; i < T; i++)
    total += summ[i];

  printf ("ряд: %.15f\nsin: %.15f\nошибка: %g\n", total, sin (x), fabs (total - sin (x)));
  return 0;
}

