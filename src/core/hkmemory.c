#include <core/hkmemory.h>

#include <core/logger.h>
#include "core/hkstring.h"

#include <platform/platform.h>

// TODO: Custom string lib
#include <string.h>
#include <stdio.h>

#if HPLATFORM_AVR && USE_PROGMEM
    #include <avr/pgmspace.h>
#endif


struct MemStatsT {
    u16 totalAllocated;
    u16 taggedAllocs[MEMT_MAX_TAGS];
}; static struct MemStatsT memStats;

static PlatformStateT* memoryPlatformState;


b8 hkInitMemory(PlatformStateT* platformState) {
    HTRACE("hkmemory.c -> hkInitMemory(void):void");
    
    memoryPlatformState = platformState;

    if(PL_IS_RDY(platformState->statusFlags, PL_MEMORY)) {
        HDEBUG("hkInitMemory(): Memory already initialized");
        return FALSE;
    }

    if(plZeroMem(&memStats, sizeof(memStats)) == NULL) {
        HERROR("hkInitMemory(): Memory could not be set!");
        PL_SET_ERR(platformState->statusFlags, PL_MEMORY);
        return FALSE;
    }
    
    PL_SET_RDY(platformState->statusFlags, PL_MEMORY);
    HDEBUG("hkInitMemory(): Memory initialized");
    return TRUE;
}

void hkStopMemory() {
    HTRACE("hkmemory.c -> hkStopMemory(void):void");

    if(!PL_IS_RDY(memoryPlatformState->statusFlags, PL_MEMORY)) {
        HDEBUG("hkAllocateMem(): Memory submodule is not initialized; nothing to stop.");
        return;
    }

    PL_SET_FLAGGED(memoryPlatformState->statusFlags, PL_MEMORY);
    return;
}


void* hkAllocateMem(u16 size, MemoryTagT tag) {
    HTRACE("hkmemory.c -> hkAllocateMem(u16, MemoryTagT):void*");
    if(!PL_IS_RDY(memoryPlatformState->statusFlags, PL_MEMORY)) {
        HWARN("hkAllocateMem(): Memory submodule is not initialized!");
        return NULL;
    }

    PL_CLEAR_FLAG(memoryPlatformState->statusFlags, PL_GENERAL_ERROR);
    PL_SET_RDY(memoryPlatformState->statusFlags, PL_MEMORY);

    if(tag == MEMT_UNKNOWN) {
        PL_SET_FLAG(memoryPlatformState->statusFlags, PL_GENERAL_ERROR);
        HWARN("hkAllocateMem(): Function was called using MEMT_UNKNOWN; Re-class this allocation.");
    }

    memStats.totalAllocated    += size;     // Total allocated memory
    memStats.taggedAllocs[tag] += size;     // Total allocated memory for given tag

    // TODO: Memory alignment
    void* block = plAllocMem(size);
    if(block == NULL) {
        HERROR("hkAllocateMem(): Memory could not be allocated!");
        PL_SET_ERR(memoryPlatformState->statusFlags, PL_MEMORY);
        return NULL;
    }

    PL_SET_RDY(memoryPlatformState->statusFlags, PL_MEMORY);
    plZeroMem(block, size);
    return block;
}

void hkFreeMem(void* block, u16 size, MemoryTagT tag) {
    HTRACE("hkmemory.c -> hkFreeMem(void*, u16, MemoryTagT):void");
    if(!PL_IS_RDY(memoryPlatformState->statusFlags, PL_MEMORY)) {
        HWARN("hkAllocateMem(): Memory submodule is not initialized!");
        return;
    }

    PL_CLEAR_FLAG(memoryPlatformState->statusFlags, PL_GENERAL_ERROR);
    PL_SET_RDY(memoryPlatformState->statusFlags, PL_MEMORY);

    if(tag == MEMT_UNKNOWN) {
        PL_SET_FLAG(memoryPlatformState->statusFlags, PL_GENERAL_ERROR);
        HWARN("hkAllocateMem(): Function was called using MEMT_UNKNOWN; Re-class this allocation.");
    }
    
    memStats.totalAllocated    -= size;     // Total allocated memory
    memStats.taggedAllocs[tag] -= size;     // Total allocated memory for given tag

    // TODO: Memory alignment
    plFreeMem(block);
    return;
}

void* hkZeroMem(void* block, u16 size) {
    HTRACE("hkmemory.c -> hkZeroMem(void*, u16):void*");

    if(!PL_IS_RDY(memoryPlatformState->statusFlags, PL_MEMORY)) {
        HWARN("hkAllocateMem(): Memory submodule is not initialized!");
        return NULL;
    } return plZeroMem(block, size);
}

void* hkCopyMem(void* dest, const void* source, u16 size) {
    HTRACE("hkmemory.c -> hkCopyMem(void*,const void*, u16):void*");

    if(!PL_IS_RDY(memoryPlatformState->statusFlags, PL_MEMORY)) {
        HWARN("hkAllocateMem(): Memory submodule is not initialized!");
        return NULL;
    } return plCopyMem(dest, source, size);
}

void* hkSetMem(void* dest, i32 value, u16 size) {
    HTRACE("hkmemory.c -> hkSetMem(void*, i32, u16):void*");

    if(!PL_IS_RDY(memoryPlatformState->statusFlags, PL_MEMORY)) {
        HWARN("hkAllocateMem(): Memory submodule is not initialized!");
        return NULL;
    } return plSetMem(dest, value, size);
}