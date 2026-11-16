#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
//-lpthread или -pthread  для успешной компиляции добавтить одну из этих опций
//Добавить вызов any_func в основную нить выполнения, чтобы распечатать 2009
void * any_func (void * arg)
{
  int a = *(int *) arg;// принудительно сказать что arg это aдрес переменной int и приравнять a к значению этой переменной
  a++;
  return (void *)(intptr_t) a;
}

int main (void)
{
  pthread_t thread;
  int parg = 2007, pdata;
  void* rx;

  if (pthread_create (&thread, NULL, &any_func, &parg) != 0)
  {
    fprintf (stderr, "Error\n");
    return 1;
  }

  pthread_join (thread, &rx);
  pdata = (int)(intptr_t) rx;
  pdata = (int)(intptr_t) any_func (&pdata);
  printf ("%d\n", pdata);

  return 0;
}в

