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
  free(mgmt);
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
void mem_free(void *p) {
  memBlock *block1, *block2;
  // free memory by changing used-flag of block to 0 which means free
  for (block1 = mgmt; (block1->next != 0); block1 = block1->next) {
    if (block1->startAddress == p) {
      block1->used = 0;
    }
  }
  for (block1 = mgmt, block2 = block1->next; (block1->next != 0) && (block2->next != 0); block1 = block2->next) {
    printf("hallo!");
  }
}
const void mem_status() {
  unsigned int used = 0;
  memBlock     *block;
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
/*
 * ┴ │ ┌ ┐└ ┘├ ┤┬ ┴ ┼
*/
const void mem_show() {
  memBlock     *block;
  unsigned int blockno  = 0;
  char         line1[1024], line2[1024], line3[1024], line4[1024], line5[1024], line6[1024], line7[1024];
  char         *line1_p = line1, *line2_p = line2, *line3_p = line3, *line4_p = line4, *line5_p = line5,
               *line6_p = line6, *line7_p = line7;
  for (block = mgmt; (block != 0); block = block->next) {
    line1_p += sprintf(line1_p, "┬───────────────────");
    line2_p += sprintf(line2_p, "│block no. %*u", 9, blockno);
    line3_p += sprintf(line3_p, "│   start: %*p", 9, block->startAddress);
    line4_p += sprintf(line4_p, "│    next: %*p", 9, block->next);
    line5_p += sprintf(line5_p, "│    size: %*u", 9, block->size);
    line6_p += sprintf(line6_p, "│    used: %*u", 9, block->used);
    line7_p += sprintf(line7_p, "┴───────────────────");
    ++blockno;
  }
  printf("%s┐\n", line1);
  printf("%s│\n", line2);
  printf("%s│\n", line3);
  printf("%s│\n", line4);
  printf("%s│\n", line5);
  printf("%s│\n", line6);
  printf("%s┘\n", line7);
}
int main() {
  mem_init(totalmem);
  mem_show();
  //mem_status();
  void *alloc1 = mem_alloc(367, __LINE__);
  printf("\nafter 0. allocation\n\n");
  //mem_status();
  mem_show();
  mem_alloc(128, __LINE__);
  printf("\nafter 1. allocation\n\n");
  //mem_status();
  mem_show();
  mem_free(alloc1);
  printf("\nafter freeing 0. allocation\n\n");
  mem_show();
  // void *alloc3 = mem_alloc(1, __LINE__);
  // void *alloc4 = mem_alloc(0, __LINE__);
  mem_cleanup();
  return 0;
}
