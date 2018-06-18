/*
 * Betriebssysteme, Sommersemester 2018
 * Nils von Nethen
 * Niklas Hansel
 */
#include <malloc.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>
bool                showFullMemory    = false;
unsigned int        sizeOfTotalMemory = 0;
void                *mem;
typedef struct node {
  unsigned int size, reqSize, line, used;
  struct node  *left, *right, *parent;
  void         *startAddress;
}                   node;
node                *root;
std::vector<node *> freeBlocks;
void traverse(node *currentNode, std::vector<node *> &allocations);
void mem_init(unsigned int totalmem);
void mem_free(void *p);
void mem_cleanup();
void *mem_alloc(unsigned int size, int line);
const void mem_status();
int main() {
  showFullMemory = true;
  mem_init(2048);// Initialisierung des Speichers
  void *p1       = mem_alloc(1024, __LINE__);
  mem_alloc(1024, __LINE__);
  void *p3 = mem_alloc(1024, __LINE__);
  mem_free(p1);
  mem_free(p3);
  p1 = mem_alloc(123, __LINE__);
  mem_alloc(512, __LINE__);
  mem_free(p1);
  mem_alloc(400, __LINE__);
  mem_status(); // Aktuelle Speicherbelegung ausgeben
  mem_cleanup();
  mem_init(4096);
  void *p4 = mem_alloc(3, __LINE__);
  mem_status();
  void *p5 = mem_alloc(1024, __LINE__);
  mem_free(p4);
  mem_status();
  mem_free(p5);
  mem_status();
  mem_cleanup();
  mem_status();
  mem_init(1234);
}
void traverse(node *currentNode, std::vector<node *> &allocations) {
  if (currentNode) {
    if (currentNode->left) {
      traverse(currentNode->left, allocations);
    }
    if (currentNode->right) {
      traverse(currentNode->right, allocations);
    }
    if (!currentNode->left && !currentNode->right) {
      allocations.push_back(currentNode);
    }
  } else {
    return;
  }
}
void mem_init(unsigned int totalmem) {
  if (!((totalmem != 0) && !(totalmem & (totalmem - 1)))) {
    std::cerr << "totalmem has to be a power of 2!" << std::endl;
    return;
  }
  sizeOfTotalMemory = totalmem;
  mem               = malloc(totalmem);
  root              = (node *) malloc(sizeof(node));
  root->size         = totalmem;
  root->reqSize      = totalmem;
  root->line         = 0;
  root->used         = 0;
  root->left         = nullptr;
  root->right        = nullptr;
  root->startAddress = mem;
  root->parent       = nullptr;
  if (!freeBlocks.empty()) {
    freeBlocks = {};
  }
  freeBlocks.push_back(root);
}
void mem_free(void *p) {
  if (p) {
    node *block = root;
    while (!((block->startAddress == p) && block->used)) {
      if (block->right->startAddress <= p) {
        block = block->right;
      } else if (block->right->startAddress > p) {
        block = block->left;
      } else {
        return;
      }
    }
    block->used = 0;
    freeBlocks.push_back(block);
    while (block->parent) {
      block = block->parent;
      // if any child has children or is used, do nothing. otherwise delete the children from the list and free memory.
      // then add the parent of the deleted children to the list of free blocks.
      if (!(block->left->left) && !(block->left->right) && !block->left->used && !(block->right->left) && !(block->right->right) && !(block->right->used)) {
        std::vector<node *>::iterator it;
        it           = std::find_if(freeBlocks.begin(), freeBlocks.end(), [&block](node *currentNode) {
          return currentNode == block->left;
        });
        freeBlocks.erase(it);
        it = std::find_if(freeBlocks.begin(), freeBlocks.end(), [&block](node *currentNode) {
          return currentNode == block->right;
        });
        freeBlocks.erase(it);
        free(block->left);
        free(block->right);
        block->left  = nullptr;
        block->right = nullptr;
        freeBlocks.push_back(block);
      }
    }
  }
}
void mem_cleanup() {
  free(mem);
  std::vector<node *> allocations;
  traverse(root, allocations);
  if (allocations.size() == 1) {
    free(*allocations.begin());
  } else {
    std::for_each(allocations.begin(), allocations.end(), [](node *currentNode) {
      if (currentNode->used) {
        mem_free(currentNode->startAddress);
      }
    });
  }
  freeBlocks        = {};
  sizeOfTotalMemory = 0;
}
void *mem_alloc(unsigned int size, int line) {
  void *alloc = nullptr;
  if (!freeBlocks.empty()) {
    auto                          sizeToAllocate = (unsigned int) pow(2, ceil(log(size) / log(2)));
    getFreeBlockFromList:
    // search for a free block in the list of free blocks until the highest possible size is reached or a sufficient free
    // block is found
    std::vector<node *>::iterator it             = freeBlocks.end();
    unsigned int                  searchFor      = sizeToAllocate;
    while (searchFor <= sizeOfTotalMemory && it == freeBlocks.end()) {
      it = std::find_if(freeBlocks.begin(), freeBlocks.end(), [&searchFor](node *currentNode) {
        return currentNode->size == searchFor;
      });
      searchFor *= 2;
    }
    if (it != freeBlocks.end()) {
      node *block = *it;
      if (block->size == sizeToAllocate && !block->right && !block->left) {
        freeBlocks.erase(it);
        block->used    = 1;
        block->line    = (unsigned int) line;
        block->reqSize = size;
        alloc = block->startAddress;
      } else if ((block->size != sizeToAllocate) && (block->right == nullptr) && (block->left == nullptr)) {
        // split the current block, then generate and assign left child
        block->left                = (node *) malloc(sizeof(node));
        block->left->startAddress  = block->startAddress;
        block->left->size          = block->size / 2;
        block->left->used          = 0;
        block->left->parent        = block;
        block->left->left          = nullptr;
        block->left->right         = nullptr;
        // generate and assign right child
        block->right               = (node *) malloc(sizeof(node));
        block->right->startAddress = ((uintptr_t *) block->startAddress) + (block->size / 2);
        block->right->size         = block->size / 2;
        block->right->used         = 0;
        block->right->parent       = block;
        block->right->left         = nullptr;
        block->right->right        = nullptr;
        // delete parent from list of free blocks (as it has children now it cannot be allocated),
        // then add both children to the list of free blocks and sort the list by startAddress so that the leftmost
        // block is found first when searching for a block
        freeBlocks.erase(it);
        freeBlocks.push_back(block->left);
        freeBlocks.push_back(block->right);
        std::sort(freeBlocks.begin(), freeBlocks.end(), [](node *a, node *b) {
          return a->startAddress < b->startAddress;
        });
        goto getFreeBlockFromList;
      }
    }
  }
  return alloc;
}
const void mem_status() {
  if (sizeOfTotalMemory == 0) {
    std::cerr << "memory has not been initialized yet!" << std::endl;
    return;
  }
  std::vector<node *> allBlocks;
  unsigned int        used = 0;
  traverse(root, allBlocks);
  std::sort(allBlocks.begin(), allBlocks.end(), [](node *a, node *b) {
    return a->startAddress < b->startAddress;
  });
  std::for_each(allBlocks.begin(), allBlocks.end(), [&used](node *block) {
    if (block->used) {
      used += block->size;
    }
  });
  printf("Start: %p End: %p\n", mem, ((uintptr_t *) mem) + sizeOfTotalMemory);
  printf("Totalmem: %#x (%u)\n", sizeOfTotalMemory, sizeOfTotalMemory);
  printf("Total allocated: %#x (%u)\n", used, used);
  unsigned int       allocationNr = 0;
  std::for_each(allBlocks.begin(), allBlocks.end(), [&allocationNr](node *block) {
    if (block->used) {
      printf("Allocation %u:\nstart: %p end: %p\nmem_alloc(%u, __LINE__); in line %u\n",
             allocationNr++,
             block->startAddress,
             ((uintptr_t *) block->startAddress) + block->size,
             block->reqSize,
             block->line);
    }
  });
  std::wstringstream line2, line3, line4, line5;
  if (showFullMemory) {
    allocationNr = 0;
    std::for_each(allBlocks.begin(), allBlocks.end(), [&](node *allocation) {
      line2 << "block no. " << std::setw(9) << allocationNr++ << " ";
      line3 << "   start: " << std::setw(9) << allocation->startAddress << " ";
      line4 << "    size: " << std::setw(9) << allocation->size << " ";
      line5 << "    used: " << std::setw(9) << allocation->used << " ";
    });
    std::wcout << line2.str() << " " << std::endl;
    std::wcout << line3.str() << " " << std::endl;
    std::wcout << line4.str() << " " << std::endl;
    std::wcout << line5.str() << " " << std::endl;
  }
}
