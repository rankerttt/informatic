#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/sem.h>

int semchange(int semid, int op, int sem_num)
{
  struct sembuf b;
  b.sem_num = sem_num;
  b.sem_op  = op;
  b.sem_flg = 0;
  if(semop(semid, &b, 1) < 0)
  {
    if(errno == EIDRM || errno == EINVAL)
      return -1;
    perror("semop");
    exit(1);
  }
  return 0;
}
int main()
{
    int ent_num;
    scanf("%d", &ent_num);
    key_t key = ftok("semaphore5.cpp",0);
    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
    int semid = semget(key, 2, IPC_CREAT | 0666);
    int* data = (int*)shmat(shmid, NULL, 0);
    *data = ent_num;
    semchange(semid,1,0);

    while (semchange(semid, -1,0) == 0)
    {
        if (*data <= 0)
        {
            semctl(semid, 0, IPC_RMID);
            shmctl(shmid, IPC_RMID, NULL);
            return 0;
        }
        (*data)--;
        printf("%d\n",*data);
        semchange(semid, 1,1);
        usleep(30000);
    }
}
