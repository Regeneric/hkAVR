#include <core/hkstring.h>
#include <core/hkmemory.h>
#include <core/logger.h>

#include <string.h>

u16 hkStrlen(const char* str) {
    HTRACE("hkstring.c -> hkstrlen(const char*):u32");

    // HACK: it will be implemented
    return strlen(str);
}

char* hkStrdup(const char* str) {
    HTRACE("hkstring.c -> hkstrdup(const char*):char*");

    u32 length = hkStrlen(str);
    
    char* copy = hkAllocateMem(length+1, MEMT_STRING);
    hkCopyMem(copy, str, length+1);

    return copy;
}

b8 hkStrcmp(const char* str0, const char* str1) {
    // HTRACE("hkstring.c -> hkstrcmp(const char*, const char*):b8");

    // HACK: it will be implemented
    return strcmp(str0, str1);
}