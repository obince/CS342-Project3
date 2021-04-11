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

struct Process {
    pid_t pid;
    struct Process* next;
};

struct Node* head = NULL;
struct Process* phead = NULL;

int sbmem_init(int segmentsize)
{
    int fd;

    printf ("sbmem init called"); // remove all printfs when you are submitting to us.

    /* DIZLA */

    fd = shm_open(SHM_NAME, O_CREAT, 0666);

    if( fd < 0)
        return -1; // Error in shared memory creation

    semap = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    ftruncate(fd, segmentsize);

    createdBefore = 1;

    void *shared_mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if( shared_mem == MAP_FAILED){
        perror("mmap err");
    }
    head = sbmem_alloc(sizeof(struct Node));

    head->size = -1;
    head->next = NULL;
    head->mem_ptr = NULL;
    head->pid = -1;

    return 0;
}

int sbmem_remove()
{
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
    /*while( head){
        struct Node temp = head->next;
        head->next = NULL;
        free(head);
        head = temp;
    }*/

    return (0);
}

int sbmem_open()
{
    int fd = shm_open(SHM_NAME, O_RDWR, /*0600*/0666);
    void *shared_mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if( shared_mem == MAP_FAILED){
        perror("mmap err");
    }
    struct Process* cur;
    int count = 0;
    for( cur = phead; cur != NULL; cur = cur->next){
        count++;
        if(cur->pid == getpid()){
            return 0;
        }
    }

    if( count == 10){
        return -1;
    }

    struct Process* pnext = sbmem_alloc(sizeof(struct Process));
    pnext->pid = getpid();
    pnext->next = NULL;
    insertProcess(pnext);

    return (0);
}

void insertProcess(struct Process* pnext){
    if(!phead){
        phead = pnext;
    } else {
        struct Process* cur = phead;
        while(!(cur->next)){
            cur = cur->next;
        }
        cur->next = pnext;
    }
}


void *sbmem_alloc(int size)
{
    if( head == NULL)
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
