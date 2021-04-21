#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>

#include "sbmem.h"

int main()
{
    int i, ret, j, number, k;
    char* p[20];
    char* ptr;
    
    printf("Hello\n");
    ret = sbmem_open();
    printf("as\n");

    if (ret == -1)
        exit (1);

    p[0] = sbmem_alloc(345);
    p[1] = sbmem_alloc(234);
    sbmem_free(p[0]);
    p[2] = sbmem_alloc(332);
    p[3] = sbmem_alloc(1234);
    p[4] = sbmem_alloc(3389);
    sbmem_free(p[3]);
    sbmem_free(p[1]);
    p[5] = sbmem_alloc(542);
    sbmem_free(p[4]);
    p[6] = sbmem_alloc(2453);
    p[7] = sbmem_alloc(234);
    sbmem_free(p[2]);
    sbmem_free(p[5]);
    sbmem_free(p[6]);
    sbmem_free(p[7]);

    /*
    for(i = 0; i < 2; i++) {
        pid_t pid = fork();
        if(pid == 0) {

            srand(time(NULL));
            ret = sbmem_open();

            if(ret == -1) {
                printf("No space left for a process. \n");
                exit(1);
            }

            for(j = 0; j < 3; j++) {
                while( (number = rand() % 4096) < 128);
                p[j] = sbmem_alloc(number);
                if(p[j] == NULL) {
                    printf("No space available for %d\n", number);
                    continue;
                }

            }

            for(k = 0; k < 3; k++) {
                printf("%d %d %d %d\n",k,k,k,k);

                if(p[k] != NULL)
                    sbmem_free(p[k]);
            }
        }
        else
            wait(NULL);
        printf("bitti\n");
    }*/
    //sbmem_close();

    return 0;
}
