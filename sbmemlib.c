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
#define TREE_MEMORY "/tree"
#define UNUSED 0
#define SPLIT 1
#define USED 2

struct TreeNode {
  struct TreeNode* right;
  struct TreeNode* left;
  struct TreeNode* parent;
  struct TreeNode* buddy;
  int count;
  int size;
  int status; //0 if unused, 1 if splitted, 2 if used
  int index;
};

int count = 0;
int delete_index = 0;
struct TreeNode* head;
char* memptr;

//Function prot
int allocate_helper(int requested_chunk_size, struct TreeNode* node);
int allocate(int requested_chunk_size);
void init(int segment_size);
void deallocate(int deletion_index);
int findIndex(int count, struct TreeNode* node);
void traverse(struct TreeNode* node);
void traverse_all();


int sbmem_init(int segmentsize)
{
    int fd;

    printf ("sbmem init called"); // remove all printfs when you are submitting to us.

    /* DIZLA */

    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if( fd < 0)
        return -1; // Error in shared memory creation

    int number_of_nodes = (segmentsize * 2) / 128;
    int treeSize = sizeof(struct TreeNode) * number_of_nodes;
    int totalSize = 2 * sizeof(int) + treeSize + segmentsize;

    semap = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    ftruncate(fd, totalSize);

    void *shared_mem = mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if( shared_mem == MAP_FAILED){
        perror("mmap err");
        return -1;
    }

    int* sizeptr = (int*) shared_mem;
    sizeptr[0] = totalSize;
    sizeptr[1] = treeSize;
    head = (struct TreeNode*) ((char*)shared_mem + 2*sizeof(int));

    head -> right = NULL;
    head -> left = NULL;
    head -> parent = NULL;
    head -> size = segment_size;
    head -> status = UNUSED;
    head -> index = 0;

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
    semap = sem_open(SEM_NAME, O_RDWR);
    int fd = shm_open(SHM_NAME, O_RDWR, /*0600*/0666);

    void *shared_mem = mmap(NULL, sizeof(int)*2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if( shared_mem == MAP_FAILED){
        perror("mmap err");
    }

    int* ptr = (int*) shared_mem;
    int totalSize = *ptr;

    void* shared_memo = mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int* shared_mem_int = (int*) shared_memo;
    char* shared_mem_ptr = (char*) shared_memo;

    head = (struct TreeNode* ) (shared_mem_ptr + 2 * sizeof(int));
    memptr = shared_mem_ptr + 2* sizeof(int) + shared_mem_int[1];


    /*
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

    /*
    struct Process* pnext = sbmem_alloc(sizeof(struct Process));
    pnext->pid = getpid();
    pnext->next = NULL;
    insertProcess(pnext);
    */
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
    int required_size = find_required_size(size);
    sem_wait(semap);
    int location = allocate(required_size);
    sem_post(semap);

    if(location == -1)
        return NULL;

    return (void*) (memptr + location);

}


void sbmem_free (void *p)
{
    char* ptr = (char*) p;
    int location = ptr - memptr;

    sem_wait(semap);
    deallocate(location);
    sem_post(semap);
}

int sbmem_close()
{
    //sem_close;
    return (0);
}

int find_required_size(int size) {
    int i;
    for(i = 4096; i >= size; i = i / 2);
    return i*2;
}

int allocate_helper(int requested_chunk_size, struct TreeNode* node) {

    struct TreeNode* left = head + (2*(node->index) + 1) * sizeof(struct TreeNode);
    struct TreeNode* right = head + (2*(node->index) + 2) * sizeof(struct TreeNode);

    if(node -> status == UNUSED) {
        if(requested_chunk_size == node->size) {
            node -> count = count;
            node -> status = USED;
            return 1;
        }
        else if(requested_chunk_size < node->size) {
            node -> status = SPLIT;

            left -> parent = node;
            left -> index = 2*(node->index) + 1;
            left -> status = UNUSED;
            left -> size = node -> size / 2;

            right -> parent = node;
            right -> index = 2*(node->index) + 2;
            right -> status = UNUSED;
            right -> size = node-> size / 2;

            node -> left = left;
            node -> right = right;

            allocate_helper(requested_chunk_size, left);
        }
        else if( requested_chunk_size > node->size) {
            count += node->size;
        }
    }
    else if(node -> status == SPLIT) {
        if(allocate_helper(requested_chunk_size, left) == 1) {
            return 1;
        }
        return allocate_helper(requested_chunk_size, right);
    }
    else if(node -> status == USED) {
        count += node -> size;
        return -1;
    }
}

/**
*   Allocate a memory segment
*   return -1 if cannot be allocated (full)
*   return the offset of allocated chunk
*/
int allocate(int requested_chunk_size) {

    if(requested_chunk_size < 64 || requested_chunk_size > 4096)
        return -1;

    count = 0;

    if(allocate_helper(requested_chunk_size, head) == 1)
        return count;

    return -1;

    //return allocate_helper(requested_chunk_size, head);
}

int findIndex(int count, struct TreeNode* node) {
    if(node -> status == USED && node -> count == count) {
        delete_index = node -> index;
        return 1;
    }
    else if(node -> status == SPLIT) {
        if(findIndex(count, node->left) == 1) {
            return 1;
        }
        return findIndex(count, node->right);
    }
    return -1;
}

void deallocate(int count) {
    if(findIndex(count, head) == -1)
        return;

    int buddy_index;
    if( delete_index % 2 == 0){
        buddy_index = delete_index - 1;
    } else {
        buddy_index = delete_index + 1;
    }

    struct TreeNode* to_be_deleted = head + delete_index * sizeof(struct TreeNode);
    struct TreeNode* buddy_node = head + buddy_index * sizeof(struct TreeNode);
    to_be_deleted->status = UNUSED;

    while( buddy_node->status == UNUSED){
        to_be_deleted = head + ((delete_index - 1) / 2) * sizeof(struct TreeNode);
        to_be_deleted->status = UNUSED;
        if( to_be_deleted->index % 2 == 0){
            buddy_index = to_be_deleted->index - 1;
        } else {
            buddy_index = to_be_deleted->index + 1;
        }
        buddy_node = head + buddy_index * sizeof(struct TreeNode);
    }

    return;
}


void traverse(struct TreeNode* node) {
    if(node -> status == USED) {
        printf("index: %d, size: %d\n", node->index, node->size);
    }
    else if( node -> status == SPLIT) {
        printf("SPLIT: index: %d, size: %d\n", node->index, node->size);
        traverse(node->left);
        traverse(node->right);
    }
}

void traverse_all() {
    traverse(head);
}

