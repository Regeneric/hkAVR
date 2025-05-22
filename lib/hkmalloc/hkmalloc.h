#pragma once

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

void* hkMalloc(size_t size);
void  hkFree(void* block);
void* hkRealloc(void* block, size_t size);


#define PACKED __attribute__((packed))

#define ALIGNMENT 2
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

// ALIGN(0) -> 0
// ALIGN(1) -> 2
// ALIGN(2) -> 2
// ALIGN(3) -> 4
// ALIGN(4) -> 4
// ALIGN(5) -> 6
// etc.

enum {
    MH_CURR_FREE = (1<<0),          // is current block free or not
    MH_PREV_FREE = (1<<1),          // is previous block free or not
    MH_LAST_ELEM = (1<<2)           // last block in the heap
    // bits 3-7 are reserved
};

typedef struct MemBlockHeaderT {
    uint8_t  flags;                 // MH_CURR_FREE | MG_PREV_FREE | MH_LAST_ELEM etc.
    uint16_t size;                  // header size + block size; multiple of alignment
    struct MemBlockHeaderT* next;   // next block
    struct MemBlockHeaderT* prev;   // previous block;
} MemBlockHeaderT;

#define HEADER_SIZE ALIGN(sizeof(MemBlockHeaderT))
// enum {HEADER_SIZE = ALIGN(sizeof(MemBlockHeaderT))};

#define MEM_STATIC_POOL_SIZE 2048
typedef struct MemStaticT {
    uint8_t  data[MEM_STATIC_POOL_SIZE];
    uint8_t* heapStart;
    uint8_t* heapEnd;
} MemStaticT;


static MemStaticT       hkMemStaticHeap;
static MemBlockHeaderT* hkMemBlockNextFree;