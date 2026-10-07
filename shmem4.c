/* Проверить совместную работу с 4.
   Написать комментарии, В ТОМ ЧИСЛЕ К ПАРАМЕТРАМ!*/
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

#define SHMEM_SIZE	4096  // размер памяти шмем
#define SH_MESSAGE	"Poglad Kota!\n" // сообщение

int main (void)
{
  int shm_id; // идентификатор памяти
  char * shm_buf; // указатель на память
  int shm_size; // размер
  struct shmid_ds ds;

  shm_id = shmget (IPC_PRIVATE,
                   SHMEM_SIZE,
  		           IPC_CREAT | IPC_EXCL | 0600); // создать память  в которую можно попасть только по заранее известному id ,
                                                //с заданным размером только для себя, ошибка если уже есть, хотя он новый будет из за IPC_Private

  if (shm_id == -1)
  {
    fprintf (stderr, "shmget() error\n");
    return 1;
  }

  shm_buf = (char *) shmat (shm_id,
                            NULL,
                            0);  // подключитть память к процессу и получить его адрес, можно читаь и писать
  if (shm_buf == -1)
  {
    fprintf (stderr, "shmat() error\n");
  	return 1;
  }

  shmctl (shm_id,
          IPC_STAT,
          &ds); //получить статистику памяти  и записать в структуру


  shm_size = ds.shm_segsz; // получить из стуктуры размер сегмента
  if (shm_size < strlen (SH_MESSAGE))
  {
  	fprintf (stderr, "error: segsize=%d\n", shm_size);
  	return 1;
  }

  strcpy (shm_buf,
          SH_MESSAGE);// написать в память сообщение

  printf ("ID: %d\n", shm_id);
  printf ("Press <Enter> to exit...");
  fgetc (stdin);

  shmdt (shm_buf);//отключить сегмент
  shmctl(shm_id,
         IPC_RMID,
         NULL);// удалить память

  return 0;
}
