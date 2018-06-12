#include <malloc.h>
typedef struct block {
  unsigned int size, reqSize, line, used;
  void         *next, *startAddress; // used: with 0 meaning free and 1 meaning used
}                  memBlock;
const unsigned int totalmem = 2048;
void               *mem;
memBlock           *mgmt;
void mem_init(unsigned int totalmem) {
  mem              = malloc(totalmem);
  mgmt             = malloc(sizeof(memBlock));
  memBlock *memory = mgmt;
  memory->startAddress = mem;
  memory->size         = totalmem;
  memory->line         = __LINE__;
  memory->used         = 0;
  memory->next         = 0;
  memory->reqSize      = totalmem;
}
void mem_cleanup() {
  free(mem);
}
void *mem_alloc(unsigned int size, int line) {
  unsigned int sizeToAllocate = 65536;
  if (size == 0 || size > totalmem) {
    return 0;
  } else if (size == 1) {
    sizeToAllocate = 1;
  } else {
    while (sizeToAllocate >= size) {
      sizeToAllocate >>= 1;
    }
    sizeToAllocate <<= 1;
  }
  memBlock          *block1          = mgmt, *block2 = 0;
  // search for block of that size, then search for block double that size etc.
  for (unsigned int currentBuddySize = 0; (sizeToAllocate << currentBuddySize) <= totalmem; ++currentBuddySize) {
    while (block1 != 0) {
      if (!block1->used) {
        if (block1->size == sizeToAllocate) {
          block1->line    = (unsigned int) line;
          block1->used    = 1;
          block1->reqSize = size;
          return block1->startAddress;
        } else if (block1->size > (sizeToAllocate << currentBuddySize)) {
          block2               = malloc(sizeof(memBlock));
          block1->size /= 2;
          block2->startAddress = block1->startAddress + block1->size;
          block2->size         = block1->size;
          block2->line         = 0;
          block2->used         = 0;
          block2->next         = block1->next;
          block2->reqSize      = 0;
          block1->next         = block2;
          // begin at first block in mgmt after dividing to scan again
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
  unsigned int used = 0;
  memBlock     *block;
  for (block        = mgmt; (block->next != 0) && block->used; block = block->next) {
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
  mem_alloc(367, __LINE__);
  printf("\nafter 0. allocation\n\n");
  mem_status();
  mem_alloc(128, __LINE__);
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
