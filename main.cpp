#include <malloc.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>
bool                mem_show    = false;
typedef struct node {
  unsigned int size, reqSize, line, used;
  struct node  *left, *right, *parent;
  void         *startAddress;
}                   node;
unsigned int        totalMemory = 0;
void                *mem;
node                *root;
std::vector<node *> freeBlocks;
void traverse(node *root, std::vector<node *> &allocations) {
  if (root) {
    if (root->left) {
      traverse(root->left, allocations);
    }
    if (root->right) {
      traverse(root->right, allocations);
    }
    if (!root->left && !root->right) {
      allocations.push_back(root);
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
  totalMemory = totalmem;
  mem         = malloc(totalmem);
  root        = (node *) malloc(sizeof(node));
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
  if (p == nullptr) {
    return;
  }
  node *block = root;
  searchForBlock:
  if ((block->startAddress == p) && block->used) {
    block->used = 0;
    freeBlocks.push_back(block);
  } else if (block->right->startAddress <= p) {
    block = block->right;
    goto searchForBlock;
  } else if (block->right->startAddress > p) {
    block = block->left;
    goto searchForBlock;
  } else {
    return;
  }
  merge:
  if (block->parent) {
    block = block->parent;
  } else {
    return;
  }
  // if any child has children or is used, do nothing. otherwise delete the children from the list and free memory.
  // then add the parent of the deleted children to the list of free blocks.
  if ((block->left->left) || (block->left->right) || block->left->used || (block->right->left) || (block->right->right)
      || block->right->used) {
    return;
  } else {
    std::vector<node *>::iterator it;
    it = std::find_if(freeBlocks.begin(), freeBlocks.end(), [&block](node *currentNode) {
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
    goto merge;
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
  freeBlocks  = {};
  totalMemory = 0;
}
void *mem_alloc(unsigned int size, int line) {
  if (freeBlocks.empty()) {
    return nullptr;
  }
  unsigned int sizeToAllocate             = 65536;
  if (size == 0 || size > totalMemory) {
    return nullptr;
  } else if (size == 1) {
    sizeToAllocate = 1;
  } else {
    while (sizeToAllocate >= size) {
      sizeToAllocate >>= 1;
    }
    sizeToAllocate <<= 1;
  }
  getFreeBlockFromList:
  // search for a free block in the list of free blocks until the highest possible size is reached or a sufficient free
  // block is found
  std::vector<node *>::iterator it;
  unsigned int                  searchFor = sizeToAllocate;
  while (true) {
    it = std::find_if(freeBlocks.begin(), freeBlocks.end(), [&searchFor](node *currentNode) {
      return currentNode->size == searchFor;
    });
    if (it == freeBlocks.end() && searchFor > totalMemory) {
      return nullptr;
    } else if (it == freeBlocks.end()) {
      searchFor *= 2;
    } else {
      break;
    }
  }
  node                          *block    = *it;
  if (block->size == sizeToAllocate && !block->right && !block->left) {
    freeBlocks.erase(it);
    block->used    = 1;
    block->line    = (unsigned int) line;
    block->reqSize = size;
    return block->startAddress;
  } else if ((block->size != sizeToAllocate) && (block->right == nullptr) && (block->left == nullptr)) {
    // split the current block
    // generate and assign left child
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
    // delete parent, add both children to list of free blocks and sort the list after startAddress
    freeBlocks.erase(it);
    freeBlocks.push_back(block->left);
    freeBlocks.push_back(block->right);
    std::sort(freeBlocks.begin(), freeBlocks.end(), [](node *a, node *b) {
      return a->startAddress < b->startAddress;
    });
    goto getFreeBlockFromList;
  } else {
    return nullptr;
  }
}
const void mem_status() {
  if (totalMemory == 0) {
    std::cerr << "memory has not been initialized yet!" << std::endl;
    return;
  }
  std::vector<node *> allocations;
  unsigned int        used = 0;
  traverse(root, allocations);
  std::sort(allocations.begin(), allocations.end(), [](node *a, node *b) {
    return a->startAddress < b->startAddress;
  });
  std::for_each(allocations.begin(), allocations.end(), [&used](node *allocation) {
    if (allocation->used) {
      used += allocation->size;
    }
  });
  printf("Start: %p End: %p\n", mem, ((uintptr_t *) mem) + totalMemory);
  printf("Totalmem: %#x (%u)\n", totalMemory, totalMemory);
  printf("Total allocated: %#x (%u)\n", used, used);
  unsigned int       allocationNr = 0;
  std::for_each(allocations.begin(), allocations.end(), [&allocationNr](node *allocation) {
    if (allocation->used) {
      printf("Allocation %u:\nstart: %p end: %p\nmem_alloc(%u, __LINE__); in line %u\n",
             allocationNr++,
             allocation->startAddress, ((uintptr_t *) allocation->startAddress) + allocation->size,
             allocation->reqSize,
             allocation->line);
    }
  });
  std::wstringstream line2, line3, line4, line5;
  if (mem_show) {
    allocationNr = 0;
    std::for_each(allocations.begin(), allocations.end(), [&](node *allocation) {
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
int main() {
  mem_show = true;
  mem_init(2048);// Initialisierung des Speichers
  void *p1 = mem_alloc(1024, __LINE__);
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
  return 0;
}
