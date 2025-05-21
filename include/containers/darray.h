#pragma once
#include <defines.h>

// Memory layout:
// u64 capacity = number of elements that can be held
// u64 length = number of elements currently contained
// u64 stride = size of each element in bytes
// void* elements

enum {
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH
};


HAPI void* _hkDarrayCreate(u16 length, u16 stride);
HAPI void  _hkDarrayDestroy(void* array);

HAPI i8    _hkDarrayFieldGet(void* array, i8 field);
HAPI void  _hkDarrayFieldSet(void* array, i8 field, u16 value);

HAPI void* _hkDarrayResize(void* array, u16 size);

HAPI void* _hkDarrayPush(void* array, const void* valuePtr);
HAPI void  _hkDarrayPop(void* array, void* dest);

HAPI void* _hkDarrayInsertAt(void* array, u16 index, void* valuePtr);
HAPI void* _hkDarrayPopAt(void* array, u16 index, void* dest);


#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR    2  

#define hkDarrayCreate(type) _hkDarrayCreate(DARRAY_DEFAULT_CAPACITY, sizeof(type))
#define hkDarrayReserve(type, capacity) _hkDarrayCreate(capacity, sizeof(type))
#define hkDarrayDestroy(array) _hkDarrayDestroy(array)

#define hkDarrayPush(array, value) {      \
    typeof(value) temp = value;         \
    array = _hkDarrayPush(array, &temp);  \
}

#define hkDarrayPushPtr(array, ptrValue)                         \
    do {                                                       \
        const void* _temp = (const void*)(ptrValue);           \
        array = _hkDarrayPush(array, &_temp);                    \
    } while(0)

#define hkDarrayPop(array, valuePtr) _hkDarrayPop(array, valuePtr)

#define hkDarrayInsertAt(array, index, value) {       \
    typeof(value) temp = value;                     \
    array = _hkDarrayInsertAt(array, index, &temp);   \
}

#define hkDarrayPopAt(array, index, valuePtr) _hkDarrayPopAt(array, index, valuePtr)
#define hkDarrayClear(array) _hkDarrayFieldSet(array, DARRAY_LENGTH, 0)
#define hkDarrayCapacity(array) _hkDarrayFieldGet(array, DARRAY_CAPACITY)
#define hkDarrayLength(array) _hkDarrayFieldGet(array, DARRAY_LENGTH)
#define hkDarrayStride(array) _hkDarrayFieldGet(array, DARRAY_STRIDE)
#define hkDarrayLengthSet(array, value) _hkDarrayFieldSet(array, DARRAY_LENGTH, value)