#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>

#include "sbmem.h"

int main()
{
    int i, ret, j, number, k;
    char* p[50];
    char* ptr;
    pid_t pid;


    for(i = 0; i < 5; i++) {
        pid = fork();

        if(pid == 0) {
            ret = sbmem_open();

            if(ret == -1) {
                printf("problema\n");
                exit(1);
            }

            for(j = 0; j < 50; j++) {
                while( (number = rand() % 4096) < 128);
                p[j] = sbmem_alloc(number);
                usleep(100);
            }

            for(j = 0; j < 50; j++) {
                if(p[j] != NULL)
                    sbmem_free(p[j]);
                usleep(100);
            }
            printf("BEN BITIRDIM! %d\n", i);
            sbmem_close();
            exit(0);
        }
    }
    for(i = 0; i < 5; i++) wait(NULL);
    printf("bitti");
    return 0;
}
