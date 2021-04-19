

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "sbmem.h"

int main()
{

    sbmem_remove();

    printf ("memory segment is removed from the system. System is clean now.\n");
    printf ("you can no longer run processes to allocate memory using the library\n");

    return (0);
}
