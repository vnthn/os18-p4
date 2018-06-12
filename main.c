#include <malloc.h>
const unsigned int totalmem = 2048;
void *mem;
struct block {
  int *startAddress, size, line, used; // with 0 meaning free and 1 meaning used
};
struct block *mgmt;
unsigned int nrOfBlocks = 0;
/**
 * Speicher allokieren
 * @param totalmem Größe des Speichers
 */
void mem_init(unsigned int totalmem) {
  // Speicher allokieren
  mem = malloc(totalmem);
  // Speicher für Verwaltung allokieren
  mgmt = malloc(sizeof(struct block));
  struct block *memory = mgmt;
  memory->startAddress = mem;
  memory->size = totalmem;
  memory->line = __LINE__;
  memory->used = 0;
  ++nrOfBlocks;
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
  struct block *memory = mgmt;
  unsigned int buddySize = 1;
  while (1) {
    for (int it = 0; it < nrOfBlocks; ++it) {
      if (buddySize > 1) {
        if (memory->size == sizeToAllocate * buddySize) {
          for (int i = 0; i < buddySize; ++i) {
            void *newBlock = memory->startAddress + memory->size / buddySize;
            struct block *newMgmt = malloc(sizeof(struct block) * nrOfBlocks + 1);
            for (int j = 0; j < nrOfBlocks; ++j) {
              *newMgmt = *mgmt;
            }
          }
        }
      } else if (memory->size == sizeToAllocate) {
        memory->line = line;
        return memory->startAddress;
      }
    }
    if (++buddySize * sizeToAllocate > totalmem)
      return 0;
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
  struct block *memory = mgmt;
  for (int i = 0; i < nrOfBlocks; ++i) {
    memory += (i) * sizeof(*memory);
    used += memory->size;
  }
  printf("Start: %p End: %p\n", mem, mem + totalmem);
  printf("Totalmem: %#x (%u)\n", totalmem, totalmem);
  printf("Total allocated: %#x (%u)\n", used, used);
  memory = mgmt;
  for (int i = 0; i < nrOfBlocks; ++i) {
    memory += (i) * sizeof(*memory);
    printf("Allocation %u:\nstart: %p end: %p\nmem_alloc(%u); in line %u",
           i,
           memory->startAddress,
           memory->startAddress + memory->size,
           memory->size,
           memory->line);
  }
}
int main() {
  mem_init(totalmem);
  mem_status();
  void *alloc1 = mem_alloc(367, __LINE__);
  void *alloc2 = mem_alloc(1024, __LINE__);
  void *alloc3 = mem_alloc(1, __LINE__);
  void *alloc4 = mem_alloc(0, __LINE__);
  mem_status();
  mem_cleanup();
  return 0;
}
