#pragma once
#include <defines.h>

HAPI u16   hkStrlen(const char* str);    // String length
HAPI char* hkStrdup(const char* str);    // String duplihkte

HAPI b8 hkStrcmp(const char* str0, const char* str1);    // String compare; Case sensitive