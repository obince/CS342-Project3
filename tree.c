#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include "tree.h"
/*
#define TREE_MEMORY "/tree"
#define NUMBER_OF_NODES 4096
#define UNUSED 0
#define SPLIT 1
#define USED 2

struct TreeNode {
  *TreeNode right;
  *TreeNode left;
  *TreeNode parent;
  *TreeNode buddy;
  int size;
  int status; //0 if unused, 1 if splitted, 2 if used
  int index;
};*/

int count = 0;
int delete_index = 0;
struct TreeNode* head;

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

int allocate(int requested_chunk_size) {

    if(requested_chunk_size < 64 || requested_chunk_size > 4096)
        return -1;

    count = 0;

    if(allocate_helper(requested_chunk_size, head) == 1)
        return count;

    return -1;

    //return allocate_helper(requested_chunk_size, head);
}

void init(int segment_size) {
    int fd, i;
    fd = shm_open(TREE_MEMORY, O_CREAT | O_RDWR, 0666);

    int size = sizeof(struct TreeNode) * NUMBER_OF_NODES;
    ftruncate(fd, size);
    void* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    head = (struct TreeNode*) ptr;
    /*
    for( i = 0; i < NUMBER_OF_NODES; i++) {

    }*/

    head -> right = NULL;
    head -> left = NULL;
    head -> parent = NULL;
    head -> size = segment_size;
    head -> status = UNUSED;
    head -> index = 0;
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
