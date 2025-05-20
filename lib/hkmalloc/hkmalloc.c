#include "hkmalloc.h"

static void hkHeapInit(void) __attribute__((constructor));
static void hkHeapInit(void) {
    hkMemStaticHeap.heapStart = &hkMemStaticHeap.data[0];
    hkMemStaticHeap.heapEnd   = &hkMemStaticHeap.data[MEM_STATIC_POOL_SIZE];

    MemBlockHeaderT* memoryBlockHeader = (MemBlockHeaderT*)hkMemStaticHeap.heapStart;
        memoryBlockHeader->size  = MEM_STATIC_POOL_SIZE;
        memoryBlockHeader->flags = MH_CURR_FREE | MH_LAST_ELEM; // is free and is last
        memoryBlockHeader->next  = NULL;
        memoryBlockHeader->prev  = NULL;
    hkMemBlockNextFree = memoryBlockHeader;
}


void* hkMalloc(size_t size) {
    size_t payload     = ALIGN(size);
    uint16_t totalSize = HEADER_SIZE + payload;
    
    // Look for a free block that will fir our data
    MemBlockHeaderT* node = hkMemBlockNextFree;
    while(node != NULL) {
        if(node->flags & MH_CURR_FREE && node->size >= totalSize) break;
        else node = node->next;
    } if(node == NULL) return NULL;   // Out of Memory

    // If rquested data size is less than (256 bytes - HEADER_SIZE), then
    // split memory chunks. Hand out to use what he needs and then create
    // new header and memory block with size (256 bytes - payload) 
    if(node->size >= totalSize + HEADER_SIZE + ALIGN(1)) {
        // Start at the byte address of `node` and move forward exactly `totalSize` bytes
        uint8_t* splitAddress = (uint8_t*)node + totalSize;

        // Same process as in: 
        // MemBlockHeaderT* memoryBlockHeader = (MemBlockHeaderT*)hkMemStaticHeap.heapStart;
        MemBlockHeaderT* splitBlockHeader = (MemBlockHeaderT*)splitAddress;
            splitBlockHeader->size = node->size - totalSize;    // Space left

            // Set MH_LAST_ELEM flag if and only if the original block was itself the last block;
            // Carry over the last‐block bit only when it was set.
            // Then set MH_CURR_FREE flag
            splitBlockHeader->flags = (node->flags & MH_LAST_ELEM) | MH_CURR_FREE;
            splitBlockHeader->next  = node->next;   // Only points to next FREE neighbour
            splitBlockHeader->prev  = node->prev;   // Only points to last FREE neighbour 

            // If block is FREE
            if(splitBlockHeader->next) splitBlockHeader->next->prev = splitBlockHeader;    // Tell the next neighbour to point at itself
            if(splitBlockHeader->prev) splitBlockHeader->prev->next = splitBlockHeader;    // Tell the prev neighbour to point at itself
            if(hkMemBlockNextFree == node) hkMemBlockNextFree = splitBlockHeader;        // Change the last free block on the list to current split

            MemBlockHeaderT* splitSuccessor = (MemBlockHeaderT*)((uint8_t*)splitBlockHeader + splitBlockHeader->size);
            if(splitSuccessor != NULL) splitSuccessor->flags |= MH_PREV_FREE;

            node->size   = totalSize;       // Change block size to hold exactly the amount of bytes passed to hmalloc()
            node->flags &= ~MH_LAST_ELEM;   // Disable MH_LAST_ELEM flag on the old node
    } else {
        // Block is not large enough to be split
        // This is `unlinking` from thre free blocks list
        if(node->next) node->next->prev = node->prev;                       // If the whole block is free, tell the prev neighbour to take it and point to current node prev neighbour
        if(node->prev) node->prev->next = node->next;                       // If the whole block is free, tell the next neighbour to take it and point to current node next neighbour
        if(hkMemBlockNextFree == node) hkMemBlockNextFree = node->next;   // If NULL it means no more free blocks
    }

    node->flags &= ~MH_CURR_FREE;   // Mark current block as used

    // If the block is the last element, mark successor as NULL
    // Else pass address to the next physical block at (node + node->size)
    MemBlockHeaderT* successor = (node->flags & MH_LAST_ELEM ? NULL : (MemBlockHeaderT*)((uint8_t*)node + node->size));
    
    // If there is another block, mark the previous one as used
    if(successor != NULL) successor->flags &= ~MH_PREV_FREE;

    // Return pointer to user data, just past the header
    return (uint8_t*)node + HEADER_SIZE;
}

void hkFree(void* block) {
    if(block == NULL) return;

    MemBlockHeaderT* node = (MemBlockHeaderT*)((uint8_t*)block - HEADER_SIZE);

    // Check if node is within bounds
    // Check if node isn't already free
    if((uint8_t*)node < hkMemStaticHeap.heapStart || (uint8_t*)node >= hkMemStaticHeap.heapEnd) return;
    if(node->flags & MH_CURR_FREE) return;

    // Set current block flag to free
    node->flags |= MH_CURR_FREE;

    node->next   = hkMemBlockNextFree;                                 // Link the old head after me
    node->prev   = NULL;                                                // I’m now at the front

    if(hkMemBlockNextFree != NULL) hkMemBlockNextFree->prev = node;   // Tell the old head that I’m before it
    hkMemBlockNextFree = node;                                         // …and *now* I am the new head

    // If the block is the last element, mark successor as NULL
    // Else pass address to the next physical block at (node + node->size)
    MemBlockHeaderT* successor = (node->flags & MH_LAST_ELEM ? NULL : (MemBlockHeaderT*)((uint8_t*)node + node->size));

    // If there is another block AFTER the current one, check if it marked as freed
    if(successor && (successor->flags & MH_CURR_FREE)) {
        if(successor->next) successor->next->prev = successor->prev;
        if(successor->prev) successor->prev->next = successor->next;
        if(hkMemBlockNextFree == successor) hkMemBlockNextFree = successor->next; 

        node->size  += successor->size;                  // Resize node free space
        node->flags |= successor->flags & MH_LAST_ELEM;  // If successor was the last element, transfer this flag to node
    }

    // If previous node is free
    if(node->flags & MH_PREV_FREE) {
        MemBlockHeaderT* predecessor = (MemBlockHeaderT*)hkMemStaticHeap.heapStart;
    
        // Iterate over all blocks from the beginning till it's not the last block
        // and it's not the current node
        while(!(predecessor->flags & MH_LAST_ELEM) && (MemBlockHeaderT*)((uint8_t*)predecessor + predecessor->size) != node) {
            predecessor = (MemBlockHeaderT*)((uint8_t*)predecessor + predecessor->size);
        }

        // Check if block just before node is free
        if((MemBlockHeaderT*)((uint8_t*)predecessor + predecessor->size) == node && (predecessor->flags & MH_CURR_FREE)) {
            // Merging current node with predecessor
            if(node->next) node->next->prev = node->prev;
            if(node->prev) node->prev->next = node->next;
            if(hkMemBlockNextFree == node) hkMemBlockNextFree = node->next;

            predecessor->size  += node->size;
            predecessor->flags |= node->flags & MH_LAST_ELEM;

            node = predecessor;
            hkMemBlockNextFree = node;  // Now predecessor is the head
        }
    }

    successor = (node->flags & MH_LAST_ELEM) ? NULL : (MemBlockHeaderT*)((uint8_t*)node + node->size);
    if(successor != NULL) {
        successor->flags = (node->flags & MH_CURR_FREE) ? (successor->flags | MH_PREV_FREE) : (successor->flags & ~MH_PREV_FREE);
    }
}