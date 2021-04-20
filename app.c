#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sbmem.h"

int main()
{
    int i, ret, j, number;
    char* p[20];

    /*
    printf("Hello\n");
    ret = sbmem_open();
    printf("as\n");

    if (ret == -1)
        exit (1);

    p = sbmem_alloc(256); // allocate space to hold 1024 characters

    printf("hebele hubele\n");
    for (i = 0; i < 256; ++i)
        p[i] = 'a'; // init all chars to ‘a’

    printf("%c\n", p[30]);

    sbmem_free(p);
    */

    for(i = 0; i < 5; i++) {
        pid_t pid = fork();
        if(pid == 0) {
            srand(time(NULL));
            ret = sbmem_open();

            if(ret == -1) {
                printf("No space left for a process. \n");
                exit(1);
            }

            for(j = 0; j < 20; j++) {
                while( (number = rand() % 4096) < 128);
                p[j] = sbmem_alloc(number);
                if(p[j] == NULL) {
                    printf("No space available for %d\n", number);
                }
                p[j][31] = 's';
                p[j][32] = 'a';
                printf("%c\n", p[j][31]);
            }

            for(j = 0; j < 20; j++) {
                sbmem_free(p[j]);
            }
            exit(0);
        }
        wait(NULL);
        printf("bitti\n");
    }
    //sbmem_close();

    return (0);
}
