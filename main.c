#include <malloc.h>
const unsigned int totalmem = 2048;
void *mem;
typedef struct block {
  int size, reqSize, line, used;
  void *next, *startAddress; // used: with 0 meaning free and 1 meaning used
} block;
block *mgmt;
/**
 * Speicher allokieren
 * @param totalmem Größe des Speichers
 */
void mem_init(unsigned int totalmem) {
  // Speicher allokieren
  mem = malloc(totalmem);
  // Speicher für Verwaltung allokieren
  mgmt = malloc(sizeof(block));
  block *memory = mgmt;
  memory->startAddress = mem;
  memory->size = totalmem;
  memory->line = __LINE__;
  memory->used = 0;
  memory->next = 0;
  memory->reqSize = totalmem;
}
/**
 * Speicher freigeben
 */
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
  block *block1 = mgmt, *block2 = 0;
  unsigned int buddySize = 1;
  // search for block of that size, then search for block double that size, then for 4 times the size, then for 8 times
  // that size etc.
  for (unsigned int currentBuddySize = 0; (sizeToAllocate << currentBuddySize) <= totalmem; ++currentBuddySize) {
    for (block1 = mgmt; (block1 != 0); block1 = block1->next) {
      if (!block1->used) {
        if (block1->size == sizeToAllocate) {
          block1->line = line;
          block1->used = 1;
          block1->reqSize = size;
          return block1->startAddress;
        } else if (block1->size == (sizeToAllocate << currentBuddySize)) {
          /* split block into two halves:
           * start1 = start1
           * start2 = start1 + size1 / 2
           * size1 = size1 / 2
           * size2 = size1
           * line1 = line
           * line2 = 0
           * used2 = used1
           * used1 = 1
           * next2 = next1
           * next1 = start2
           */
          block2 = malloc(sizeof(block));
          block1->size /= 2;
          block2->startAddress = block1->startAddress + block1->size;
          block2->size = block1->size;
          block1->line = line;
          block2->line = 0;
          block2->used = block1->used;
          block1->used = 1;
          block2->next = block1->next;
          block1->next = block2;
          block1->reqSize = size;
          block2->reqSize = 0;
          return block1->startAddress;
        }
      }
    }
  }
}
void mem_free(void *p) {
}
/**
 * Beispiel-Ausgabe:
 * Start: 0x08abc008 End: 0x08abc808
 * Totalmem: 0x00000800 (2048)
 * Total allocated: 0x00000800 (2048)
 * Allocation 0:
 * start: 0x08abc008 end: 0x08abc408
 * mem_alloc (1024); in line 202
 * Allocation 1:
 * start: 0x08abc408 end: 0x08abc808
 * mem_alloc (1024); in line 826
 */
const void mem_status() {
  int used = 0;
  block *block;
  for (block = mgmt; (block->next != 0); block = block->next) {
    used += block->size;
  }
  printf("Start: %p End: %p\n", mem, mem + totalmem);
  printf("Totalmem: %#x (%u)\n", totalmem, totalmem);
  printf("Total allocated: %#x (%u)\n", used, used);
  unsigned int allocationNr = 0;
  for (block = mgmt; (block->next != 0); block = block->next) {
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
  void *alloc3 = mem_alloc(1, __LINE__);
  void *alloc4 = mem_alloc(0, __LINE__);
  mem_cleanup();
  return 0;
}
