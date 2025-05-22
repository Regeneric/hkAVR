#include <core/hkmemory.h>

#include <core/logger.h>
#include "core/hkstring.h"

#include <platform/platform.h>

// TODO: Custom string lib
#include <string.h>
#include <stdio.h>

#if HPLATFORM_AVR && HK_USE_PROGMEM
    #include <avr/pgmspace.h>
#endif


#if HK_MEM_HEAVY_DBG
struct MemStatsT {
    u16 totalAllocated;
    u16 taggedAllocs[MEMT_MAX_TAGS];
}; static struct MemStatsT memStats;
#endif

static PlatformStateT* memoryPlatformState;


b8 hkInitMemory(PlatformStateT* platformState) {
    HTRACE("hkmemory.c -> hkInitMemory(void):void");
    
    memoryPlatformState = platformState;

    if(PL_IS_RDY(platformState->statusFlags, PL_MEMORY)) {
        HDEBUG("hkInitMemory(): Memory already initialized");
        return FALSE;
    }

    #if HK_MEM_HEAVY_DBG
        if(plZeroMem(&memStats, sizeof(memStats)) == NULL) {
            HERROR("hkInitMemory(): Memory could not be set!");
            PL_SET_ERR(platformState->statusFlags, PL_MEMORY);
            return FALSE;
        }
    #endif
    
    PL_SET_RDY(platformState->statusFlags, PL_MEMORY);
    HDEBUG("hkInitMemory(): Memory initialized");
    return TRUE;
}

void hkStopMemory() {
    HTRACE("hkmemory.c -> hkStopMemory(void):void");
    HDEBUG("hkStopMemory(): Stopping memory subsystem.");

    if(!PL_IS_RDY(memoryPlatformState->statusFlags, PL_MEMORY)) {
        HDEBUG("hkAllocateMem(): Memory submodule is not initialized; nothing to stop.");
        return;
    }

    #if HK_MEM_HEAVY_DBG
        plZeroMem(&memStats, sizeof(memStats));
    #endif

    PL_SET_FLAGGED(memoryPlatformState->statusFlags, PL_MEMORY);
    HINFO("Memory subsystem has been stopped.");
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

    #if HK_MEM_HEAVY_DBG
        memStats.totalAllocated    += size;     // Total allocated memory
        memStats.taggedAllocs[tag] += size;     // Total allocated memory for given tag
    #endif

    // TODO: Memory alignment
    void* block = plAllocMem(size);
    if(block == NULL) {
        HERROR("hkAllocateMem(): Memory could not be allocated!");
        PL_SET_FLAG(memoryPlatformState->statusFlags, PL_GENERAL_ERROR);
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
    
    #if HK_MEM_HEAVY_DBG
        memStats.totalAllocated    -= size;     // Total allocated memory
        memStats.taggedAllocs[tag] -= size;     // Total allocated memory for given tag
    #endif

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


#if HK_MEM_HEAVY_DBG
    #if HK_USE_PROGMEM
        static const char memoryTagStrings[MEMT_MAX_TAGS][15] PROGMEM = {
            "UNKNOWN     ",
            "ARRAY       ",
            "DARRAY      ",
            "DICT        ",
            "RING_QUEUE  ",
            "BST         ",
            "STRING      ",
            "APPLICATION ",
            "JOB         ",
            "RENDERER    ",
            "ENTITY      "
        };
    #else
        static const char* memoryTagStrings[MEMT_MAX_TAGS] = {
            "UNKNOWN     ",
            "ARRAY       ",
            "DARRAY      ",
            "DICT        ",
            "RING_QUEUE  ",
            "BST         ",
            "STRING      ",
            "APPLICATION ",
            "JOB         ",
            "RENDERER    ",
            "ENTITY      "
        };
    #endif


char* hkDebugMemoryUsage() {
    const u32 GiB = 1UL << 30;
    const u32 MiB = 1UL << 20;
    const u16 KiB = 1U  << 10;

    char buffer[512];
    u16 bufSz = sizeof(buffer);
    u16 offset = snprintf(buffer, bufSz, "System memory use (tagged):\n");
    if(offset >= bufSz) offset = bufSz - 1;
    
    for(u32 i = 0; i < MEMT_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        f32 amount = 1.0f;
        u16 used = memStats.taggedAllocs[i];

        if(used >= GiB) {
            unit[0] = 'G';
            amount = used / (f32)GiB;
        } else if(used >= MiB) {
            unit[0] = 'M';
            amount = used / (f32)MiB;
        } else if(used >= KiB) {
            unit[0] = 'K';
            amount = used / (f32)KiB;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (f32)used;
        }
        
        #if HK_USE_PROGMEM
            char currentMemoryTagString[15];
            strcpy_P(currentMemoryTagString, memoryTagStrings[i]);
        #else
            const char* currentMemoryTagString = memoryTagStrings[i];
        #endif

        u16 rem = bufSz > offset ? bufSz - offset : 0;
        if(rem == 0) break;

        u32 length = snprintf(buffer+offset, rem, "  %s: %.2f%s\n", currentMemoryTagString, amount, unit);
        if(length < 0) break;
        if((u16)length >= rem) break;
        offset += (u16)length;
    }

    char* outString = hkStrdup(buffer);
    if(!outString) return '\0';
    return outString;
}
#endif