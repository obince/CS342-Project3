#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sbmem.h"

int main()
{
    int i, ret, j, number, k;
    char* p, *p2, *p3, *p4, *p5;

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

            //for(j = 0; j < 4; j++) {
                while( (number = rand() % 4096) < 128);
                printf("pp\n");
                p = sbmem_alloc(number);
                while( (number = rand() % 4096) < 128);
                printf("pp2\n");
                p2 = sbmem_alloc(number);
                while( (number = rand() % 4096) < 128);
                printf("pp3\n");
                p3 = sbmem_alloc(number);
                while( (number = rand() % 4096) < 128);
                printf("pp4\n");
                p4 = sbmem_alloc(number);

                if(p != NULL) {
                    //p[j][31] = 's';
                    //p[j][32] = 'a';
                    //printf("%c\n", p[j][31]);
                    printf("p\n");
                    sbmem_deneme(p);
                }
                else {
                    printf("No space available for %d\n", number);
                }
                if(p != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
                    printf("p2\n");
                    sbmem_free(p2);
                    sbmem_deneme(p2);
                    printf("p3\n");
                    sbmem_free(p3);
                    sbmem_deneme(p3);
                    printf("p4\n");
                    sbmem_free(p4);
                    sbmem_deneme(p4);
                }

            //}
            /*
            for(k = 0; k < 10; k++) {
                printf("%d.......\n", k);
                if(p[k] == NULL) continue;

                if(p[k] != NULL) {
                    sbmem_free(p[k]);
                }
            }*/
            //exit(0);
        //}
        //wait(NULL);
        printf("bitti\n");
    //}
    //sbmem_close();

    return (0);
}
