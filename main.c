#include <malloc.h>
const unsigned int totalmem = 2048;
void *mem;
struct buddy {
  int *startAddress, size, line, used; // with 0 meaning free and 1 meaning used
};
struct buddy *mgmt;
unsigned int nrOfBuddies = 0;
/**
 * Speicher allokieren
 * @param totalmem Größe des Speichers
 */
void mem_init(unsigned int totalmem) {
  // Speicher allokieren
  mem = malloc(totalmem);
  // Speicher für Verwaltung allokieren
  mgmt = malloc(sizeof(struct buddy));
  struct buddy *memory = mgmt;
  memory->startAddress = mem;
  memory->size = totalmem;
  memory->line = __LINE__;
  memory->used = 0;
  ++nrOfBuddies;
}
/**
 * Speicher freigeben
 */
void mem_cleanup() {
  free(mem);
}
void *mem_alloc(unsigned int size, int line) {
  struct buddy *memory = mgmt;
  unsigned int sizeToAllocate;
  if (size == 0) {
    return 0;
  } else {
    sizeToAllocate = 65536;
    while (!(sizeToAllocate & size)) {
      sizeToAllocate >>= (unsigned int) 1;
    }
    if (sizeToAllocate < size) {
      sizeToAllocate <<= (unsigned int) 1;
    }
  }
  return malloc(sizeToAllocate);
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
  struct buddy *memory = mgmt;
  for (int i = 0; i < nrOfBuddies; ++i) {
    memory += (i) * sizeof(*memory);
    used += memory->size;
  }
  printf("Start: %p End: %p\n", mem, mem + totalmem);
  printf("Totalmem: %#x (%u)\n", totalmem, totalmem);
  printf("Total allocated: %#x (%u)\n", used, used);
  memory = mgmt;
  for (int i = 0; i < nrOfBuddies; ++i) {
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
  void *alloc2 = mem_alloc(2048, __LINE__);
  void *alloc3 = mem_alloc(1, __LINE__);
  void *alloc4 = mem_alloc(0, __LINE__);
  mem_status();
  mem_cleanup();
  return 0;
}
