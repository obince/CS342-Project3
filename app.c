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

    //for(i = 0; i < 5; i++) {
        //pid_t pid = fork();
        //if(pid == 0) {

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
                p[j][31] = 's';
                p[j][32] = 'a';
                printf("%c\n", p[j][31]);

            }
        //}
        //wait(NULL);
        /*
        char p[10];
        p[0] = '\0';
        if( p[0] == NULL)
            printf("null\n");
        printf("bitti\n");*/
    //}
    //sbmem_close();

    return (0);
}
