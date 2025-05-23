#include <containers/darray.h>
#include <core/hkmemory.h>
#include <core/logger.h>

void* _hkDarrayCreate(u16 length, u16 stride) {
    HTRACE("darray.c -> _hkDarrayCreate(u16, u16):void*");

    u16  headerSize = DARRAY_FIELD_LENGTH * sizeof(u16);
    u16  arraySize  = length * stride;
    u16* newArray   = hkAllocateMem(headerSize+arraySize, MEMT_DARRAY);
    
    hkZeroMem(newArray, headerSize+arraySize);
    newArray[DARRAY_CAPACITY] = length;
    newArray[DARRAY_LENGTH]   = 0;
    newArray[DARRAY_STRIDE]   = stride;

    return (void*)(newArray + DARRAY_FIELD_LENGTH);
}

void _hkDarrayDestroy(void* array) {
    HTRACE("darray.c -> _hkDarrayDestroy(void*):void");

    u16* header = (u16*)array - DARRAY_FIELD_LENGTH;
    u16  headerSize = DARRAY_FIELD_LENGTH * sizeof(u16);
    u16  totalSize  = headerSize + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];     // header + array size * element size

    hkFreeMem(header, totalSize, MEMT_DARRAY);
    return;
}

i8 _hkDarrayFieldGet(void* array, i8 field) {
    // HTRACE("darray.c -> _hkDarrayFieldGet(void*, i8):i8");

    u16* header = (u16*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _hkDarrayFieldSet(void* array, i8 field, u16 value) {
    HTRACE("darray.c -> _hkDarrayFieldSet(void*, i8, u16):void");

    u16* header = (u16*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
    return;
}

void* _hkDarrayResize(void* array, u16 size) {
    HTRACE("darray.c -> _hkDarrayResize(void*, u16):void*");

    u16 length = hkDarrayLength(array);
    u16 stride = hkDarrayStride(array);

    void* temp = _hkDarrayCreate(DARRAY_RESIZE_FACTOR * hkDarrayCapacity(array), stride);
    hkCopyMem(temp, array, length*stride);

    _hkDarrayFieldSet(temp, DARRAY_LENGTH, length);
    _hkDarrayDestroy(array);
    return temp;
}

void* _hkDarrayPush(void* array, const void* valuePtr) {
    HTRACE("darray.c -> _hkDarrayPush(void*, const void*):void*");

    u16 length = hkDarrayLength(array);
    u16 stride = hkDarrayStride(array);
    if(length >= hkDarrayCapacity(array)) array = _hkDarrayResize(array, length+stride);

    u16* addr = (u16*)array;
    addr += (length*stride);
    hkCopyMem((void*)addr, valuePtr, stride);
    _hkDarrayFieldSet(array, DARRAY_LENGTH, length+1);

    return array;
}

void _hkDarrayPop(void* array, void* dest) {
    HTRACE("darray.c -> _hkDarrayPop(void*, void*):void");

    u16 length = hkDarrayLength(array);
    u16 stride = hkDarrayStride(array);
    
    u16* addr = (u16*)array;
    addr += ((length+1)*stride);
    hkCopyMem(dest, (void*)addr, stride);
    _hkDarrayFieldSet(array, DARRAY_LENGTH, length-1);

    return;
}

void* _hkDarrayInsertAt(void* array, u16 index, void* valuePtr) {
    HTRACE("darray.c -> _hkDarrayInsertAt(void*, u16, void*):void*");

    u16 length = hkDarrayLength(array);
    u16 stride = hkDarrayStride(array);
    
    if(index >= length) {
        HWARN("_hkDarrayInsertAt(): Index outside of the bound of this array! Length: %i, index: %i", length, index);
        return array;
    } 
    if(length >= hkDarrayCapacity(array)) array = _hkDarrayResize(array, length+stride);

    // Move everything from addr and move it to addr+1
    u16* addr = (u16*)array;
    if(index != length-1) {
        hkCopyMem(
            (void*)(addr + ((index+1)*stride)),
            (void*)(addr + (index*stride)),
            stride * (length-index)
        );
    }

    hkCopyMem((void*)(addr + (index*stride)), valuePtr, stride);
    _hkDarrayFieldSet(array, DARRAY_LENGTH, length+1);

    return array;
}

void* _hkDarrayPopAt(void* array, u16 index, void* dest) {
    HTRACE("darray.c -> _hkDarrayPopAt(void*, u16, void*):void*");

    u16 length = hkDarrayLength(array);
    u16 stride = hkDarrayStride(array);
    if(index >= length) {
        HWARN("_hkDarrayPopAt(): Index outside of the bound of this array! Length: %i, index: %i", length, index);
        return array;
    }

    u16* addr = (u16*)array;
    hkCopyMem(dest, (void*)(addr + (index*stride)), stride);
    
    // Move everything from addr+1 and move it to addr
    if(index != length-1) {
        hkCopyMem(
            (void*)(addr + (index*stride)),
            (void*)(addr + ((index+1)*stride)),
            stride * (length-index)
        );
    }

    _hkDarrayFieldSet(array, DARRAY_LENGTH, length-1);
    return array;
}