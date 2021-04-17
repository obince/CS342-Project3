#define TREE_MEMORY "/tree"
#define NUMBER_OF_NODES 4096
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

int allocate_helper(int requested_chunk_size, struct TreeNode* node);

int allocate(int requested_chunk_size);

void init(int segment_size);

void deallocate(int deletion_index);

void traverse_all();
