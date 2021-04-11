#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>

// Define a name for your shared memory; you can give any name that start with a slash character; it will be like a filename.
#define SHM_NAME "/name_project3"

// Define semaphore(s)
#define SEM_NAME "/name_semap"
sem_t* semap;

// Define your stuctures and variables.
struct Node {
    int size;
    struct Node* next;
    void* mem_ptr;
    pid_t pid;
};

int createdBefore = 0;

struct Node* head;

int sbmem_init(int segmentsize)
{
    int fd;

    printf ("sbmem init called"); // remove all printfs when you are submitting to us.

    if(createdBefore)
        sbmem_remove();

    fd = shm_open(SHM_NAME, O_CREAT, 0666);

    if( fd < 0)
        return -1; // Error in shared memory creation

    semap = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    ftruncate(fd, segmentsize);

    createdBefore = 1;

    head = malloc(sizeof(struct Node));

    return 0;
}

int sbmem_remove()
{
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);

    return (0);
}

int sbmem_open()
{

    return (0);
}


void *sbmem_alloc (int size)
{
    return (NULL);
}


void sbmem_free (void *p)
{


}

int sbmem_close()
{
    //sem_close;
    return (0);
}
