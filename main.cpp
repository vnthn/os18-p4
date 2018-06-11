#include <iostream>
void mem_init(unsigned int totalmem);
void mem_cleanup();
void *mem_alloc(unsigned int size, int line);
void mem_free(void *p);
const void mem_status();
const unsigned int totalmem = 2048;
void *mem;
char *f = (char *) "f";
char *u = (char *) "u";
int main() {
  // Speicher allokieren
  mem_init(totalmem);
  mem_status();
  mem_cleanup();
  return 0;
}
/**
 * Speicher allokieren
 * @param totalmem Größe des Speichers
 */
void mem_init(unsigned int totalmem) {
  mem = malloc(totalmem);
  void *pVoid = mem;
  for (int i = 0; i < totalmem; i++) {
    *((char *) mem + i * sizeof(*f)) = *f;
  }
}
/**
 * Speicher freigeben
 */
void mem_cleanup() {
  free(mem);
}
void *mem_alloc(unsigned int size, int line) {
  return (char *) mem + size;
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
  int free = 0;
  int used = 0;
  for (int i = 0; i < totalmem; i++) {
    if (*((char *) mem + i * sizeof(*f)) == *f) {
      free++;
    } else if (*((char *) mem + i * sizeof(*f)) == *u) {
      used++;
    }
  }
  std::cout << "Start: " << mem << " End: " << mem + totalmem << std::endl << "Totalmem: " << std::hex << "0x"
            << totalmem << std::dec << " (" << totalmem << ")" << std::endl << "Total allocated: " << std::hex << "0x"
            << used << std::dec << " (" << used << ")" << std::endl;
}