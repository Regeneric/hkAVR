#include <string.h>

#include <containers/darray.h>
#include <core/hkmemory.h>
#include <core/logger.h>

#define HEADER_SIZE_BYTES  (DARRAY_FIELD_LENGTH * sizeof(u16))

void* _hkDarrayCreate(u16 length, u16 stride) {
    HTRACE("darray.c -> _hkDarrayCreate(u16, u16):void*");

    size_t arrayBytes = (size_t)length * stride;
    size_t totalBytes = HEADER_SIZE_BYTES + arrayBytes;
    
    u8* newArray = hkAllocateMem(totalBytes, MEMT_DARRAY);
    if(newArray == NULL) {
        HERROR("_hkDarrayCreate(): Array could not be created!");
        return NULL;
    } else hkZeroMem(newArray, totalBytes);

    u16* header = (u16*)newArray;
    header[DARRAY_CAPACITY] = length;
    header[DARRAY_LENGTH]   = 0;
    header[DARRAY_STRIDE]   = stride;

    return (void*)(newArray + HEADER_SIZE_BYTES);

    // u16  headerSize = DARRAY_FIELD_LENGTH * sizeof(u16);
    // u16  arraySize  = length * stride;
    // u16* newArray   = hkAllocateMem(headerSize+arraySize, MEMT_DARRAY);
    
    // hkZeroMem(newArray, headerSize+arraySize);
    // newArray[DARRAY_CAPACITY] = length;
    // newArray[DARRAY_LENGTH]   = 0;
    // newArray[DARRAY_STRIDE]   = stride;

    // return (void*)(newArray + DARRAY_FIELD_LENGTH);
}

void _hkDarrayDestroy(void* array) {
    HTRACE("darray.c -> _hkDarrayDestroy(void*):void");

    u8* data    = (u8*)array;
    u8* base    = data - HEADER_SIZE_BYTES;
    u16* header = (u16*)base;
    
    size_t arrayBytes = (size_t)header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    size_t totalBytes = HEADER_SIZE_BYTES + arrayBytes;

    hkFreeMem(base, totalBytes, MEMT_DARRAY);

    // u16* header = (u16*)array - DARRAY_FIELD_LENGTH;
    // u16  headerSize = DARRAY_FIELD_LENGTH * sizeof(u16);
    // u16  totalSize  = headerSize + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];     // header + array size * element size

    // hkFreeMem(header, totalSize, MEMT_DARRAY);
    // return;
}

i8 _hkDarrayFieldGet(void* array, i8 field) {
    // HTRACE("darray.c -> _hkDarrayFieldGet(void*, i8):i8");

    u8*  base   = (u8*)array - HEADER_SIZE_BYTES;
    u16* header = (u16*)base;
    return (i8)header[field]; 

    // u16* header = (u16*)array - DARRAY_FIELD_LENGTH;
    // return header[field];
}

void _hkDarrayFieldSet(void* array, i8 field, u16 value) {
    HTRACE("darray.c -> _hkDarrayFieldSet(void*, i8, u16):void");

    u8*  base   = (u8*)array - HEADER_SIZE_BYTES;
    u16* header = (u16*)base;
    header[field] = value; 
}

void* _hkDarrayResize(void* array, u16 size) {
    HTRACE("darray.c -> _hkDarrayResize(void*, u16):void*");

    u16 capacity = hkDarrayCapacity(array);
    u16 stride   = hkDarrayStride(array);
    u16 length   = hkDarrayLength(array);

    // Pick whichever is larger
    u16 targetCapactiy = size > (capacity*DARRAY_RESIZE_FACTOR) ? size : (capacity*DARRAY_RESIZE_FACTOR);

    void* newArray = _hkDarrayCreate(targetCapactiy, stride);
    if(newArray == NULL) {
        HERROR("_hkDarrayResize(): Array could not be resized!");
        return NULL;
    }

    size_t newSize = (size_t)length * stride;
    hkCopyMem(newArray, array, newSize);

    _hkDarrayFieldSet(newArray, DARRAY_LENGTH, length);
    _hkDarrayDestroy(array);
    return newArray;


    // u16 length = hkDarrayLength(array);
    // u16 stride = hkDarrayStride(array);

    // void* temp = _hkDarrayCreate(DARRAY_RESIZE_FACTOR * hkDarrayCapacity(array), stride);
    // hkCopyMem(temp, array, length*stride);

    // _hkDarrayFieldSet(temp, DARRAY_LENGTH, length);
    // _hkDarrayDestroy(array);
    // return temp;
}

void* _hkDarrayPush(void* array, const void* valuePtr) {
    HTRACE("darray.c -> _hkDarrayPush(void*, const void*):void*");

    u16 capacity = hkDarrayCapacity(array);
    u16 stride   = hkDarrayStride(array);
    u16 length   = hkDarrayLength(array);

    if(length >= capacity) {
        array = _hkDarrayResize(array, length+1);
        if(array == NULL) {
            HERROR("_hkDarrayPush(): Array could not be resized!");
            return NULL;
        }
    }

    u8* data = (u8*)array;
    u8* dest = data + (size_t)length * stride;
    hkCopyMem(dest, valuePtr, stride);

    _hkDarrayFieldSet(array, DARRAY_LENGTH, length+1);
    return array;

    // u16 length = hkDarrayLength(array);
    // u16 stride = hkDarrayStride(array);
    // if(length >= hkDarrayCapacity(array)) array = _hkDarrayResize(array, length+stride);

    // u16* addr = (u16*)array;
    // addr += (length*stride);
    // hkCopyMem((void*)addr, valuePtr, stride);
    // _hkDarrayFieldSet(array, DARRAY_LENGTH, length+1);

    // return array;
}

void _hkDarrayPop(void* array, void* dest) {
    HTRACE("darray.c -> _hkDarrayPop(void*, void*):void");

    u16 length = hkDarrayLength(array);
    if(length == 0) return;

    u16 stride = hkDarrayStride(array);
    u8* data   = (u8*)array;
    u8* source = data + (size_t)(length-1) * stride;
    
    hkCopyMem(dest, source, stride);
    _hkDarrayFieldSet(array, DARRAY_LENGTH, length-1);

    return;

    // u16 length = hkDarrayLength(array);
    // u16 stride = hkDarrayStride(array);
    
    // u16* addr = (u16*)array;
    // addr += ((length+1)*stride);
    // hkCopyMem(dest, (void*)addr, stride);
    // _hkDarrayFieldSet(array, DARRAY_LENGTH, length-1);

    // return;
}

void* _hkDarrayInsertAt(void* array, u16 index, void* valuePtr) {
    HTRACE("darray.c -> _hkDarrayInsertAt(void*, u16, void*):void*");

    u16 capacity = hkDarrayCapacity(array);
    u16 stride   = hkDarrayStride(array);
    u16 length   = hkDarrayLength(array);

    if(index > length) {
        HWARN("_hkDarrayInsertAt(): index %u out of bounds (len %u)", index, length);
        return array;
    }

    if(index >= capacity) {
        array = _hkDarrayResize(array, length+1);
        if(array == NULL) {
            HERROR("_hkDarrayInsertAt(): Array could not be resized!");
            return NULL;
        }
    }

    u8* data  = (u8*)array;
    u8* start = data + (size_t)index * stride;

    size_t temp = (size_t)(length-index) * stride;
    memmove(start+stride, start, temp);

    hkCopyMem(start, valuePtr, stride);
    _hkDarrayFieldSet(array, DARRAY_LENGTH, length+1);
    return array;

    // u16 length = hkDarrayLength(array);
    // u16 stride = hkDarrayStride(array);
    
    // if(index >= length) {
    //     HWARN("_hkDarrayInsertAt(): Index outside of the bound of this array! Length: %i, index: %i", length, index);
    //     return array;
    // } 
    // if(length >= hkDarrayCapacity(array)) array = _hkDarrayResize(array, length+stride);

    // // Move everything from addr and move it to addr+1
    // u16* addr = (u16*)array;
    // if(index != length-1) {
    //     hkCopyMem(
    //         (void*)(addr + ((index+1)*stride)),
    //         (void*)(addr + (index*stride)),
    //         stride * (length-index)
    //     );
    // }

    // hkCopyMem((void*)(addr + (index*stride)), valuePtr, stride);
    // _hkDarrayFieldSet(array, DARRAY_LENGTH, length+1);

    // return array;
}

void* _hkDarrayPopAt(void* array, u16 index, void* dest) {
    HTRACE("darray.c -> _hkDarrayPopAt(void*, u16, void*):void*");

    u16 length = hkDarrayLength(array);
    u16 stride = hkDarrayStride(array);
    if(index >= length) {
        HWARN("_hkDarrayPopAt(): index %u out of bounds (len %u)", index, length);
        return array;
    }

    u8* data = (u8*)array;
    u8* src  = data + (size_t)index * stride;
    hkCopyMem(dest, src, stride);

    size_t moveBytes = (size_t)(length - index-1) * stride;
    memmove(src, src + stride, moveBytes);

    _hkDarrayFieldSet(array, DARRAY_LENGTH, length-1);
    return array;

    // u16 length = hkDarrayLength(array);
    // u16 stride = hkDarrayStride(array);
    // if(index >= length) {
    //     HWARN("_hkDarrayPopAt(): Index outside of the bound of this array! Length: %i, index: %i", length, index);
    //     return array;
    // }

    // u16* addr = (u16*)array;
    // hkCopyMem(dest, (void*)(addr + (index*stride)), stride);
    
    // // Move everything from addr+1 and move it to addr
    // if(index != length-1) {
    //     hkCopyMem(
    //         (void*)(addr + (index*stride)),
    //         (void*)(addr + ((index+1)*stride)),
    //         stride * (length-index)
    //     );
    // }

    // _hkDarrayFieldSet(array, DARRAY_LENGTH, length-1);
    // return array;
}