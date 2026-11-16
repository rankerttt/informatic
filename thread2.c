#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
/*
 * потому что ее значение записывается в указатель void* и не исчезает
 * Потому что нельзя intptr_t от числа double , поэтому используют настоящий адрес
*/
double a;

void * any_func (void * arg)
{
  printf ("arg %p\n", arg);
  printf ("*arg %f\n", *(double *)arg);
  //double a = *(double *)arg;
  a = *(double *)arg;
  a++;
  printf ("a %f\n", a);
  double * pa = &a;
  printf ("pa %p\n", pa);
  printf ("*pa %f\n", *pa);
  return pa; // возвращаем указатель
}

int main (void)
{
  pthread_t thread;
  double data = 2007;
  void * pp = &data;
  // указатель pdata должен указывать на доступную память иначе может быть сегментация
  int q = 0;
  void * pq = (void *)&q;
  void ** pdata = pq;// = 0;
  printf ("pp %p\n", pp);
  printf (" * pp %f\n", *((double *)pp));
  if (pthread_create
       (
        // указатель на переменную, в которую будет записан идентификатор созданного потока
        &thread,
        // дополнительные параметры создаваемого потока
        NULL,
        // указатель на потоковую функцию
        &any_func,
        //нетипизированный указатель на параметры, передаваемые потоку
        pp
       )
       != 0
     )
  {
    fprintf (stderr, "Error\n");
    return 1;
  }
  printf ("2 pp %p\n", pp);
  printf ("2 * pp %f\n", *((double *)pp));

  pthread_join (
    // идентификатор потока
    thread,
    /* If  retval  is  not NULL, then pthread_join() copies the exit status of
       the target thread (i.e., the value that the target thread  supplied  to
       pthread_exit(3)) into the location pointed to by retval.  If the target
       thread was canceled, then PTHREAD_CANCELED is placed  in  the  location
       pointed to by retval. */
    // нетипизированный указатель на копию! нетипизированного указателя с результатом работы потока
    pdata
                );
  //значение указателя на копию указателя
  printf ("pdata %p\n", pdata);
  // указатель на a
  printf ("* pdata %p\n", *pdata);
  // значение a
  printf (" ** pdata %f\n", (double)(**(double **)pdata));
  a++;
  printf (" ** pdata %f\n", (double)(**(double **)pdata));

  return 0;
}
