#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define FILENAME "buffer.txt"

void inputm(int *upid){
    char buffen[256];
    while (true){
        int r = scanf("%d:%255[^\n]", upid, buffen);
        if (r == EOF) break;
        if (r == 2){
            FILE *in = fopen(FILENAME, "a");
            if (in){
                fprintf(in, "%d:%s\n", *upid, buffen);
                fclose(in);
            }
        } else {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
        usleep(10000);
    }
}

void scanner(int *mpid){
    while (true){
        char buff[256];
        int spid;
        FILE *inn = fopen(FILENAME, "r");
        if (inn){
            if (fgets(buff, sizeof(buff), inn) &&
                sscanf(buff, "%d", &spid) == 1 &&
                spid == *mpid){
                printf("%s", buff);
                fflush(stdout);
                fclose(inn);

                FILE *iz = fopen(FILENAME, "w");
                if (iz) fclose(iz);
            } else {
                fclose(inn);
            }
        }
        usleep(10000);
    }
}

int main(){
    int mypid = getpid();
    printf("your pid %d\n", mypid);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0){
        perror("fork");
        return 1;
    }
    if (pid == 0){
        scanner(&mypid);
        return 0;
    }
    inputm(&mypid);
    return 0;
}

