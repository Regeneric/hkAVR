#pragma once
#include <defines.h>

#include <platform/platform.h>

typedef enum MemoryTagT {
    MEMT_UNKNOWN,
    MEMT_ARRAY,
    MEMT_DARRAY,
    MEMT_DICT,
    MEMT_RING_QUEUE,
    MEMT_BST,
    MEMT_STRING,
    MEMT_APPLICATION,
    MEMT_JOB,
    MEMT_RENDERER,
    MEMT_ENTITY,

    MEMT_MAX_TAGS
} MemoryTagT;

b8   hkInitMemory(PlatformStateT* platformState);
void hkStopMemory(void);

HAPI void* hkAllocateMem(u16 size, MemoryTagT tag);
HAPI void  hkFreeMem(void* block, u16 size, MemoryTagT tag);
HAPI void* hkZeroMem(void* block, u16 size);
HAPI void* hkCopyMem(void* dest, const void* source, u16 size);
HAPI void* hkSetMem(void* dest, i32 value, u16 size);

// #define HK_MEM_HEAVY_DBG TRUE
HAPI char* hkDebugMemoryUsage();