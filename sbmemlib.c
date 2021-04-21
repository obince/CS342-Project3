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
#define PT_SEM_NAME "/name_pt"
sem_t* semap;
sem_t* pt_semap;

// Define your stuctures and variables.
#define TREE_MEMORY "/tree"
#define UNUSED 0
#define SPLIT 1
#define USED 2
#define INDEX_SIZE 18
#define TWO_POWER(i) (1 << (i))
#define OVER_HEAD_SEGMENT_SIZE 18 * sizeof(long) + sizeof(struct ProcessTable)
#define OVER_HEAD_BLOCK_SIZE sizeof(struct OverHead)

struct OverHead {
    int size;
    long next;
    long prev;
    int tag;
};

struct ProcessTable {
    pid_t processes[10];
    int count;
    int frag[10];
    int allocated[10];
};
int SGM_SIZE;
long* freelists;
void* shared_mem;
struct ProcessTable* pt;
struct stat sbuf;
int count = 0;
int delete_index = 0;
struct TreeNode* head;
void* memptr;
int tots;
int process_i;
int index_size;
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
            pt->count = pt->count + 1;
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
            pt->count = pt->count - 1;
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

    printf("sbmem init called\n"); // remove all printfs when you are submitting to us.
    printf("hop1\n");
    /* DIZLA */

    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if( fd < 0)
        return -1; // Error in shared memory creation

    int totalSize = OVER_HEAD_SEGMENT_SIZE + segmentsize;

    int segment_index = log2_custom(segmentsize);

    semap = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    pt_semap = sem_open(PT_SEM_NAME, O_CREAT | O_RDWR, 0666, 1);
    ftruncate(fd, totalSize);

    void *shared_mem = mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    printf("Shared mem addr: %ld\n", (long)shared_mem);
    if( shared_mem == MAP_FAILED){
        perror("mmap err");
        return -1;
    }

    char* p = (char*) shared_mem;

    for( i = 0; i < totalSize; i++) {
        p[i] = '\0';
    }

    long* ptr = (long*) shared_mem;
    for(i = 0; i < 18; i++) {
        ptr[i] = 0;

        if(i == segment_index) {
            ptr[i] = (OVER_HEAD_SEGMENT_SIZE);
            printf("Buna atadım %ld\n", (long) ((char*) shared_mem + OVER_HEAD_SEGMENT_SIZE));
        }
    }

    pt = (struct ProcessTable*) ((char*) shared_mem + 18 * sizeof(long));
    pt_init(pt);

    p = p + OVER_HEAD_SEGMENT_SIZE;

    printf("baslangic: %ld\n", (long)p);

    struct OverHead* o_ptr = (struct OverHead* ) p;

    o_ptr->size = segmentsize;
    // o_ptr->status = 0;
    o_ptr->next = 0;
    o_ptr->prev = 0;
    o_ptr->tag = 1;

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
    pt = (struct ProcessTable*) ((char*) shared_mem + 18 * sizeof(long));

    pt_remove(pt);
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
    sem_unlink(PT_SEM_NAME);
    return (0);
}

int sbmem_open()
{
    int fd = shm_open(SHM_NAME, O_RDWR, 0600);

    fstat(fd, &sbuf);

    SGM_SIZE = sbuf.st_size - (OVER_HEAD_SEGMENT_SIZE);

    shared_mem = mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    //printf("Shared mem addr: %ld\n", (long int)shared_mem);
    if( shared_mem == MAP_FAILED){
        perror("mmap err");
    }

    freelists = (long*) shared_mem;

    pt = (struct ProcessTable*) ((char*) shared_mem + 18 * sizeof(long));

    semap = sem_open(SEM_NAME, O_RDWR);
    pt_semap = sem_open(PT_SEM_NAME, O_RDWR);

    sem_wait(pt_semap);
    if(pt_available(pt)) {
        pt_open(pt);
    }
    else{
        sem_post(pt_semap);
        return -1;
    }
    sem_post(pt_semap);
    return (0);
}

int find_required_size(int size) {
    size = size + OVER_HEAD_BLOCK_SIZE;
    return log2_custom(size);
}

void* sbmem_alloc(int size)
{
    sem_wait(semap);
    int required_size = 1 << (find_required_size(size));

    void* ptr = alloc(required_size);

    if(ptr == NULL){
        sem_post(semap);
        return NULL;
    }

    struct OverHead* o_ptr = (struct OverHead*) ptr;
    o_ptr->tag = 0;

    sem_wait(pt_semap);
    pt->allocated[process_i] += o_ptr->size;
    pt->frag[process_i] += (o_ptr->size) - size;
    sem_post(pt_semap);

    printf("Block size: %d, Requested size: %d\n", o_ptr->size, size);

    ptr =(void*) ((char*) ptr + sizeof(struct OverHead));
    sem_post(semap);

    return ptr;
}

void sbmem_free (void *p)
{
    sem_wait(semap);
    p =(void*) ((char*) p - sizeof(struct OverHead));
    struct OverHead* o_ptr = (struct OverHead*) p;
    o_ptr->tag = 1;
    dealloc(p);
    sem_post(semap);

}

int sbmem_close()
{
    sem_wait(pt_semap);
    pt_close(pt);
    sem_post(pt_semap);
    sem_close(pt_semap);

    sem_close(semap);
    return (0);
}

void* findbuddy(void* block) {
    struct OverHead* ohead = (struct OverHead*) block;
    long buddyNo = ((long) block - (long)((char*) shared_mem + 18 * sizeof(long))) / ohead->size;
    if( buddyNo % 2 == 0){
        return (void*) ((long) block + ohead->size);
    } else {
        return (void*) ((long) block - ohead->size);
    }

/*
    if(ohead->status == 0) {
        return (void *) ((char*) block + ohead->size);
    }
    else
        return (void *) ((char*) block - ohead->size);*/
}

void* alloc(int size) {
    int i;
    for(i = 0; TWO_POWER(i) < size; i++);

    if( i > INDEX_SIZE) {
        return NULL;
    }
    else if(freelists[i] != 0) {
        void* block;
        block = (void*) (freelists[i] + (long) shared_mem);

        struct OverHead* block_ptr = (struct OverHead*) block;
        freelists[i] = block_ptr->next;

        printf("Mem address: %ld\n", (long) block);

        if(block_ptr->next != 0){
            void* next_block = (void*) (block_ptr->next + (long) shared_mem);
            struct OverHead* next_block_ptr = (struct OverHead*) next_block;
            next_block_ptr->prev = 0;
        }

        block_ptr->next=0;
        block_ptr->prev=0;

        return block;
    }
    else {
        void* block;
        void* buddy;
        block = alloc(TWO_POWER(i+1));

        if(block != NULL) {

            //Bölme işlemi yap
            //buddyi free yap listeye koy
            //blocku dön

            struct OverHead* block_ptr = (struct OverHead*) block;
            block_ptr->size = block_ptr->size / 2;
            printf("BOL: %d, %d\n",block_ptr->size, i);

            buddy = findbuddy(block);
            struct OverHead* buddy_ptr = (struct OverHead*) buddy;

            buddy_ptr -> next = freelists[i];
            buddy_ptr -> prev = 0;

            if(buddy_ptr -> next != 0) {
                void* next_block = (void*) (buddy_ptr->next + (long) shared_mem);
                struct OverHead* next_block_ptr = (struct OverHead*) next_block;

                next_block_ptr -> prev = (long) buddy - (long) shared_mem;
            }

            freelists[i] = (long) buddy - (long) shared_mem;

            //buddy_ptr->status = block_ptr->status + 1;
            //block_ptr->status = 0;
            buddy_ptr->size = block_ptr->size;
            buddy_ptr -> tag = 1;
            block_ptr -> tag = 1;
            block_ptr->next = 0;
            block_ptr->prev = 0; // change
        }
        return block;
    }
}


void dealloc(void* block) {
    struct OverHead* block_ptr = (struct OverHead*) block;

    int i;
    int size = block_ptr->size;
    printf("Mem address: %ld\n", (long) block);
    printf("birlestirs %d\n", size);
    //void** p;
    void* buddy;
    for(i = 0; TWO_POWER(i) < size; i++);


    //burayı elimle girdim 32 kbtan fazla olursa dönsün sıkıntı çıkmasın diye
    if(i == 15) {
        freelists[i] = (long) block - (long) shared_mem;
        block_ptr->prev = 0;
        block_ptr->next = 0;
        block_ptr->tag = 1;
        return;
    }

    buddy = findbuddy(block);
    struct OverHead* buddy_ptr = (struct OverHead*) buddy;


    // burada sizeın segmentsizea ulaşıp ulaşmadığını kontrol etmek lazım
    if(buddy_ptr->tag == 0 || (buddy_ptr->tag == 1 && buddy_ptr->size != block_ptr->size)) {
        
        block_ptr->prev = 0;
        block_ptr->tag = 1;

        if(freelists[i] == 0) {
            freelists[i] = (long) block - (long) shared_mem;
            block_ptr->next = 0;
        }
        else { //boş değilse
            void* next_block = (void*) (freelists[i] + (long) shared_mem);
            struct OverHead* next_block_ptr = (struct OverHead*) next_block;

            block_ptr->next = freelists[i];
            freelists[i] = (long) block - (long) shared_mem;
            next_block_ptr->prev = (long) block - (long) shared_mem;
        }
    }
    else if (buddy_ptr-> tag == 1 && buddy_ptr->size == block_ptr->size) {  //buddy availablesa birleştir.

        //buddy i freelistten çıkar
        if((void*) (freelists[i] + (long) shared_mem) == buddy) { //eğer freelist buddyi tutuyorsa
            freelists[i] = buddy_ptr->next;
            if(buddy_ptr-> next != 0) { //NULL değilse
                void* next_block = (void*) (buddy_ptr->next + (long) shared_mem);
                struct OverHead* next_block_ptr = (struct OverHead*) next_block;
                next_block_ptr->prev = 0;
            }
        }
        else { //eğer freelist buddyi tutmuyorsa
            void* prev_block = (void*) (buddy_ptr->prev + (long) shared_mem);
            struct OverHead* prev_block_ptr = (struct OverHead*) prev_block;

            prev_block_ptr->next = buddy_ptr->next;
            buddy_ptr ->next = 0;
            buddy_ptr ->prev = 0;
        }

        if(block < buddy) {
            block_ptr->size = block_ptr->size * 2;
            block_ptr->tag = 1;
            dealloc(block);
        }
        else {
            buddy_ptr->size = buddy_ptr->size * 2;
            buddy_ptr->tag = 1;
            dealloc(buddy);
        }
    }
}


