#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <semaphore.h>


// Define a name for your shared memory; you can give any name that start with a slash character; it will be like a filename.
#define SHM_NAME "/name_project3"

// Define semaphore(s)
#define SEM_NAME "/name_semap"
sem_t* semap;

// Define your stuctures and variables.
#define TREE_MEMORY "/tree"
#define UNUSED 0
#define SPLIT 1
#define USED 2
#define INDEX_SIZE 18
#define TWO_POWER(i) (1 << (i))
#define OVER_HEAD_SEGMENT_SIZE 18 * sizeof(void*) + sizeof(struct ProcessTable)
#define OVER_HEAD_BLOCK_SIZE sizeof(void*) + sizeof(struct OverHead)

struct OverHead {
    int size;
    int status;
};

struct ProcessTable {
    pid_t processes[10];
    int count;
    int frag[10];
    int allocated[10];
};

void** freelists;
void* shared_mem;
struct ProcessTable* pt;
struct stat sbuf;
int count = 0;
int delete_index = 0;
struct TreeNode* head;
void* memptr;
int tots;
int process_i;

//Function prot
void dealloc(void* block);
void* alloc(int size);
int find_required_size(int size);
void* findbuddy(void* block);


int processExists(struct ProcessTable* pt) {
    int i;
    pid_t pid = getpid();

    for( i = 0; i < 10; i++) {
        if(pt->processes[i] == pid)
            return 1;
    }
    return 0;
}

int pt_available(struct ProcessTable* pt) {
    int i;
    pid_t pid = getpid();

    if(count >= 10)
        return 0;

    for( i = 0; i < 10; i++) {
        if( pt->processes[i] == pid) {
            return 0;
        }
    }
    return 1;
}

int pt_open(struct ProcessTable* pt) {
    int i;
    pid_t pid = getpid();

    for( i = 0; i < 10; i++) {
        if( pt->processes[i] == -1) {
            count++;
            pt->processes[i] = pid;
            process_i = i;
            return 1;
        }
    }
    return 0;
}

void pt_init(struct ProcessTable* pt) {
    int i;
    pt->count = 0;
    for( i = 0; i < 10; i++) {
        pt->processes[i] = -1;
        pt->allocated[i] = 0;
        pt->frag[i] = 0;
    }
}

void pt_close(struct ProcessTable* pt) {
    int i;
    pid_t pid = getpid();

    for( i = 0; i < 10; i++) {
        if(pt->processes[i] == pid) {
            pt->processes[i] = -1;
            return;
        }
    }
}

void pt_remove(struct ProcessTable* pt) {
    int i;
    long total_frag = 0;
    long total_allocated = 0;

    for( i = 0; i < 10; i++) {

        total_allocated += pt->allocated[i];
        total_frag += pt->frag[i];

        pt->allocated[i] = 0;
        pt->frag[i] = 0;
        pt->processes[i] = -1;
    }

    printf("Number of processes: %d\n", pt->count);
    printf("Total fragmentation amount in bytes: %ld\n", total_frag);
    printf("Total allocation amount in bytes: %ld\n", total_allocated);
    printf("Fragmentation allocation ratio: %.3lf\n", (total_frag * 1.0) / total_allocated);
    pt->count = 0;
}

int log2_custom(int number) {
    int i;
    for(i = 0; TWO_POWER(i) < number; i++);
    return i;
}


int sbmem_init(int segmentsize)
{
    int fd, i;

    printf ("sbmem init called\n"); // remove all printfs when you are submitting to us.
    printf("hop1\n");
    /* DIZLA */

    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if( fd < 0)
        return -1; // Error in shared memory creation

    int totalSize = OVER_HEAD_SEGMENT_SIZE + segmentsize;

    int segment_index = log2_custom(segmentsize);

    semap = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    ftruncate(fd, totalSize);

    void *shared_mem = mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    printf("Shared mem addr: %ld", (long int)shared_mem);
    if( shared_mem == MAP_FAILED){
        perror("mmap err");
        return -1;
    }

    char* p = (char*) shared_mem;

    for( i = 0; i < totalSize; i++) {
        p[i] = 0;
    }

    void* ptr = shared_mem;
    for(i = 0; i < 18; i++) {
        *(void**)ptr = NULL;

        if(i == segment_index) {
            *(void**)ptr = (void*) (OVER_HEAD_SEGMENT_SIZE);
            printf("Buna atadım %ld\n", (long int) ((char*) shared_mem + OVER_HEAD_SEGMENT_SIZE));
        }

        ptr = (char*)ptr + sizeof(void*);
    }

    pt = (struct ProcessTable*) ((char*) shared_mem + 18 * sizeof(void*));
    pt_init(pt);

    p = p + OVER_HEAD_SEGMENT_SIZE;

    *(void**)p = NULL;
    printf("baslangic: %ld\n", (long int)p);
    p = p + sizeof(void*);
    struct OverHead* o_ptr = (struct OverHead* ) p;

    o_ptr->size = segmentsize;
    o_ptr->status = 0;

    return 0;
}

int sbmem_remove()
{
    int fd = shm_open(SHM_NAME, O_RDWR, 0600);

    fstat(fd, &sbuf);

    shared_mem = mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    //printf("Shared mem addr: %ld\n", (long int)shared_mem);
    if( shared_mem == MAP_FAILED){
        perror("mmap err");
    }
    pt = (struct ProcessTable*) ((char*) shared_mem + 18 * sizeof(void*));
    pt_remove(pt);
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
    return (0);
}

int sbmem_open()
{
    int fd = shm_open(SHM_NAME, O_RDWR, 0600);

    fstat(fd, &sbuf);

    shared_mem = mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    //printf("Shared mem addr: %ld\n", (long int)shared_mem);
    if( shared_mem == MAP_FAILED){
        perror("mmap err");
    }

    freelists = (void**)shared_mem;
    pt = (struct ProcessTable*) ((char*) shared_mem + 18 * sizeof(void*));

    semap = sem_open(SEM_NAME, O_RDWR);

    if(pt_available(pt)) {
        pt_open(pt);
    }
    else
        return -1;
    return (0);
}

int find_required_size(int size) {
    size = size + OVER_HEAD_BLOCK_SIZE;
    return log2_custom(size);
}

void* sbmem_alloc(int size)
{
    int required_size = 1 << (find_required_size(size));

    sem_wait(semap);
    void* ptr = alloc(required_size);
    sem_post(semap);

    if(ptr == NULL)
        return NULL;
    sem_wait(semap);
    struct OverHead* o_ptr = (struct OverHead*) ((char*) ptr + sizeof(void*));

    pt->allocated[process_i] += o_ptr->size;
    pt->frag[process_i] += (o_ptr->size) - size;
    printf("Block size: %d, Requested size: %d\n", o_ptr->size, size);

    ptr =(void*) ((char*) ptr + sizeof(void*) + sizeof(struct OverHead));
    sem_post(semap);
    return ptr;
}

void sbmem_free (void *p)
{
    sem_wait(semap);
    p =(void*) ((char*) p - sizeof(void*) - sizeof(struct OverHead));
    dealloc(p);
    sem_post(semap);
}

int sbmem_close()
{
    sem_close(semap);
    pt_close(pt);
    return (0);
}

void* findbuddy(void* block) {
    char * ptr = (char *) block;
    char* over_ptr = ptr + sizeof(void*);
    struct OverHead* ohead = (struct OverHead *) over_ptr;

    if(ohead->status == 0) {
        return (void *) (ptr + ohead->size);
    }
    else
        return (void *) (ptr - ohead->size);
}

void* alloc(int size) {
    int i;
    for(i = 0; TWO_POWER(i) < size; i++);

    if( i >= INDEX_SIZE) {
        printf("No space exists\n");
        return NULL;
    }
    else if( freelists[i] != NULL) {
        void* block;
        block = (void*) ((long int) freelists[i] + (long) shared_mem); // maybe cast to char* and add smem and cast to void*
        //printf("else if Girdim %ld\n", (long int) block);
        struct OverHead* block_ptr = (struct OverHead*) ((char*) block + sizeof(void*));
        freelists[i] = *((void**)((long) freelists[i] + (long) shared_mem)); // maybe cast to char* and subtract smem and cast to void*
        //printf("ELSE IF BIRADER!! %d, %d\n",block_ptr->size, i);
        *(void**)block = NULL;
        return block;
    }
    else {
        void* block;
        void* buddy;
        //printf("else girdim %d\n", i);
        block = alloc(TWO_POWER(i+1));

        if(block != NULL) {
            struct OverHead* block_ptr = (struct OverHead*) ((char*) block + sizeof(void*));
            block_ptr->size = block_ptr->size / 2;
            printf("BOL: %d, %d\n",block_ptr->size, i);
            buddy = findbuddy(block);

            *(void**) buddy = freelists[i];
            freelists[i] = (void*) ((long) buddy - (long) shared_mem);

            struct OverHead* buddy_ptr = (struct OverHead*) ((char*) buddy + sizeof(void*));

            buddy_ptr->status = block_ptr->status + 1;
            block_ptr->status = 0;
            buddy_ptr->size = block_ptr->size;

        }
        return block;
    }
}

void dealloc(void* block) {
    struct OverHead* block_ptr = (struct OverHead*) ((char*) block + sizeof(void*));

    int i;
    int size = block_ptr->size;
    printf("birlestirs %d\n", size);
    void** p;
    void* buddy;
    for(i = 0; TWO_POWER(i) < size; i++);

    buddy = findbuddy(block);
    p = &freelists[i];

    while((*p != NULL) && ((void*)((long)*p + (long) shared_mem) != buddy)) p = (void**)((void*)((long)*p + (long) shared_mem));

    if((void*)((long)*p + (long) shared_mem) != buddy) {
        *(void**) block = freelists[i];
        freelists[i] = (void*) ((long) block - (long) shared_mem);
    }
    else {
        *p = *(void**) buddy;
        struct OverHead* buddy_ptr = (struct OverHead*) ((char*) buddy + sizeof(void*));
        if(block_ptr->status == 0) {
            buddy_ptr->size = buddy_ptr->size * 2;
            block_ptr->size = block_ptr->size * 2;
            block_ptr->status = buddy_ptr->status - 1;
            dealloc(block);
        }
        else {
            buddy_ptr->size = buddy_ptr->size * 2;
            block_ptr->size = block_ptr->size * 2;
            buddy_ptr->status = block_ptr->status - 1;
            dealloc(buddy);
        }
    }
}

void sbmem_deneme(void* ptr) {
    struct OverHead* o_ptr = (struct OverHead*) ((char*) ptr - sizeof(struct OverHead));
    printf("%d\n", o_ptr->size);
}
