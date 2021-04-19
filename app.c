#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "sbmem.h"

int main()
{
    int i, ret;
    char *p;
    //sbmem_init(32768);
    printf("Hello\n");
    ret = sbmem_open();
    printf("as\n");

    if (ret == -1)
        exit (1);

    p = sbmem_alloc(256); // allocate space to hold 1024 characters


    for (i = 0; i < 256; ++i)
        p[i] = 'a'; // init all chars to ‘a’

    printf("%c", p[30]);
    sbmem_free(p);

    //sbmem_close();
    //sbmem_remove();
    return (0);
}
