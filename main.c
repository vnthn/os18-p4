#include <malloc.h>
typedef struct block {
  int size, reqSize, line, used;
  void *next, *startAddress; // used: with 0 meaning free and 1 meaning used
} memBlock;
const unsigned int totalmem = 2048;
void *mem;
memBlock *mgmt;
void mem_init(unsigned int totalmem) {
  mem = malloc(totalmem);
  mgmt = malloc(sizeof(memBlock));
  memBlock *memory = mgmt;
  *memory = (memBlock) {
      .startAddress = mem,
      .size = totalmem,
      .line = __LINE__,
      .used = 0,
      .next = 0,
      .reqSize = totalmem};
}
void mem_cleanup() {
  free(mem);
}
void *mem_alloc(unsigned int size, int line) {
  unsigned int sizeToAllocate;
  if (size == 0) {
    return 0;
  } else if (size == 1) {
    sizeToAllocate = 1;
  } else if (size > totalmem) {
    return 0;
  } else {
    sizeToAllocate = 65536;
    while (sizeToAllocate >= size) {
      sizeToAllocate >>= 1;
    }
    sizeToAllocate <<= 1;
  }
  // search for block of that size
  memBlock *block1 = mgmt, *block2 = 0;
  // search for block of that size, then search for block double that size, then for 4 times the size, then for 8 times
  // that size etc.
  for (unsigned int currentBuddySize = 0; (sizeToAllocate << currentBuddySize) <= totalmem; ++currentBuddySize) {
    while (block1 != 0) {
      if (!block1->used) {
        if (block1->size == sizeToAllocate) {
          block1->line = line;
          block1->used = 1;
          block1->reqSize = size;
          return block1->startAddress;
        } else if (block1->size > (sizeToAllocate << currentBuddySize)) {
          block2 = malloc(sizeof(memBlock));
          // block2 = block1->next;
          // block2 = malloc(sizeof(memBlock));
          block1->startAddress = block1->startAddress;
          block1->size /= 2;
          *block2 = (memBlock) {
              .startAddress = block1->startAddress + block1->size,
              .size = block1->size,
              .line = 0,
              .used = 0,
              .next = block1->next,
              .reqSize = 0};
          block1->next = block2;
          // begin at start after dividing to scan again
          block1 = mgmt;
        }
      } else {
        block1 = block1->next;
      }
    }
  }
}
/*
void mem_free(void *p) {
}
 */
const void mem_status() {
  int used = 0;
  memBlock *block;
  for (block = mgmt; (block->next != 0) && block->used; block = block->next) {
    used += block->size;
  }
  printf("Start: %p End: %p\n", mem, mem + totalmem);
  printf("Totalmem: %#x (%u)\n", totalmem, totalmem);
  printf("Total allocated: %#x (%u)\n", used, used);
  unsigned int allocationNr = 0;
  for (block = mgmt; (block->next != 0) && block->used; block = block->next) {
    printf("Allocation %u:\nstart: %p end: %p\nmem_alloc(%u, __LINE__); in line %u\n",
           allocationNr++,
           block->startAddress,
           block->startAddress + block->size,
           block->reqSize,
           block->line);
  }
}
int main() {
  mem_init(totalmem);
  mem_status();
  void *alloc1 = mem_alloc(367, __LINE__);
  printf("\nafter 0. allocation\n\n");
  mem_status();
  void *alloc2 = mem_alloc(128, __LINE__);
  printf("\nafter 1. allocation\n\n");
  mem_status();
  // mem_free(alloc1);
  // printf("\nafter freeing 0. allocation\n\n");
  // mem_status();
  // void *alloc3 = mem_alloc(1, __LINE__);
  // void *alloc4 = mem_alloc(0, __LINE__);
  mem_cleanup();
  return 0;
}
