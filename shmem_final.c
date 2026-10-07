//программа создает память с списком имен чтобы исключить одинаковые имена, далее при вводе получателя открывает
// чат между ними и есть отедльная команда для выхода
//важно чтобы имена людей были суммарно из расзых букв, потому что если имена отличаются перестановкой то ключ будет одинаковый

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

#define SHM_MAXSIZE (1 << 20)
#define DELAY (10*1000)
#define OFFSET 100
#define KEY_NAMES 000111

int name_exists(const char *base_names, const char *name)
{
    size_t name_len = strlen(name);
    const char *p = base_names;

    while (*p != '\0')
    {
        const char *line_end = strchr(p, '\n');
        size_t line_len = line_end ? (size_t)(line_end - p) : strlen(p);

        if (line_len == name_len && strncmp(p, name, name_len) == 0)
            return 1;

        if (!line_end)
            break;
        p = line_end + 1;
    }
    return 0;
}

int name_sum(const char *s)
{
    int sum = 0;
    while (*s)
        sum += (unsigned char)*s++;
    return sum;
}

int chat_number(const char *a, const char *b)
{
    return name_sum(a) + name_sum(b);
}


int main()
{
    char myname[64] = {0};
    int base_id = shmget(KEY_NAMES, SHM_MAXSIZE, IPC_CREAT | 0666);
    if (base_id < 0) { perror("shmget base"); exit(1); }

    char *base_names = (char*)shmat(base_id, NULL, 0);
    if (base_names == (char*)-1) { perror("shmat base"); exit(1); }


    while(myname[0] == '\0')
    {
        printf("Enter your name: ");
        if (fgets(myname, sizeof(myname), stdin) == NULL) {
            printf("\nExit captured.\n");
            return 0;
        }

        myname[strcspn(myname, "\n")] = '\0';

        if (strlen(myname) == 0) continue;

        if (name_exists(base_names, myname) == 1) {
            printf("Name already exists! Try another one.\n");
            myname[0] = '\0';
            fflush(stdout);
        } else {
            int lenn = strlen(base_names);

            if (lenn + (int)strlen(myname) + 2 > SHM_MAXSIZE) {
                printf("No space left for new names!\n");
                myname[0] = '\0';
                continue;
            }
            sprintf(base_names + lenn, "%s\n", myname);
            break;
        }
    }


    while (true)
    {
        char partname[64] = {0};

        while(partname[0] == '\0')
        {
            printf("Enter partner name (or '/exit' to quit program): ");
            if (fgets(partname, sizeof(partname), stdin) == NULL) {
                return 0;
            }

            partname[strcspn(partname, "\n")] = '\0';

            if (strcmp(partname, "/exit") == 0) {
                printf("Goodbye!\n");
                return 0;
            }

            if (strlen(partname) == 0) continue;

            if (name_exists(base_names, partname) == 0) {
                printf("User '%s' does not exist in the system!\n", partname);
                partname[0] = '\0';
                fflush(stdout);
            }
        }

        key_t key_h = chat_number(partname, myname);
        int shmid = shmget(key_h, SHM_MAXSIZE, IPC_CREAT | 0666);
        if (shmid < 0) { perror("shmget chat"); continue; }

        // ВАЖНО: читаем nattch ДО собственного shmat
        struct shmid_ds buf;
        if (shmctl(shmid, IPC_STAT, &buf) < 0) {
            perror("shmctl");
            continue;
        }

        if (buf.shm_nattch >= 4) {
            printf("Only tet-a-tet chat is supported! Room is full.\n");
            continue;
        }

        int numb = (buf.shm_nattch == 0) ? 1 : 2;

        char* shm_adr = (char*)shmat(shmid, NULL, 0);
        if (shm_adr == (char*)-1) {
            perror("shmat chat");
            continue;
        }

        if (numb == 1) {
            shm_adr[0] = 0;
        }

        pid_t process_pid = fork();
        if (process_pid < 0) {
            perror("fork");
            shmdt(shm_adr);
            continue;
        }

        if (process_pid == 0) // ПОТОМОК: Чтение сообщений
        {
            while (1)
            {
                if (shm_adr[0] != 0 && shm_adr[0] != (numb + '0'))
                {
                    if (strncmp(shm_adr + OFFSET, "/exit", 5) == 0) {
                        printf("\n%s left the chat.\n", partname);
                        shm_adr[0] = 0;
                        break;
                    }
                    printf("\n--> %s\n", shm_adr + OFFSET);
                    printf("You: ");
                    fflush(stdout);
                    shm_adr[0] = 0;
                }
                usleep(DELAY);
            }
            shmdt(shm_adr);
            exit(0);
        }
        else // РОДИТЕЛЬ: Отправка сообщений
        {
            char line[1024];
            bool forced_exit = false;

            printf("[Вы вошли в чат с %s. Напишите /exit для выхода]\n", partname);

            while (true)
            {
                printf("You: ");
                fflush(stdout);

                if (!fgets(line, sizeof line, stdin)) break;

                line[strcspn(line, "\n")] = 0;
                if (line[0] == 0) continue;

                if (waitpid(process_pid, NULL, WNOHANG) > 0) {
                    forced_exit = true;
                    break;
                }

                if (shm_adr[0] != 0) {
                    printf("Собеседник ещё не прочитал прошлое сообщение, повторите позже.\n");
                    continue;
                }

                strcpy(shm_adr + OFFSET, line);
                shm_adr[0] = numb + '0';

                if (strcmp(line, "/exit") == 0) {
                    usleep(DELAY * 2);
                    break;
                }
            }

            if (!forced_exit) {
                kill(process_pid, SIGTERM);
                waitpid(process_pid, NULL, 0);
            }

            shmdt(shm_adr);

            if (shmctl(shmid, IPC_STAT, &buf) == 0 && buf.shm_nattch == 0) {
                shmctl(shmid, IPC_RMID, NULL);
            }

            printf("\n[Вы вышли из комнаты чата]\n\n");
        }
    }

    return 0;
}
