#pragma once
#include <defines.h>

#define PL_LOGGING        0
#define PL_EVENT          2
#define PL_INPUT          4
#define PL_MEMORY         6
#define PL_USART          8
#define PL_TIMER          10
#define PL_RESERVED_1     12
#define PL_RESERVED_2     14
#define PL_MALLOC_WARN   (UINT32_C(1) << 28)
#define PL_GENERAL_ERROR (UINT32_C(1) << 29)
#define PL_RESERVED_4    (UINT32_C(1) << 30)
#define PL_ALL_INIT_OK   (UINT32_C(1) << 31)

#define PL_STATE_MASK 0x03  // 2-bit mask
#define GET_SUBSYS_STATE(flags, shift) (((flags) >> (shift)) & PL_STATE_MASK)
#define SET_SUBSYS_STATE(flags, shift, state) ((flags) = ((flags) & ~(PL_STATE_MASK << (shift))) | ((state & PL_STATE_MASK) << (shift)))

#define PL_IS_RDY(flags, shift)     (GET_SUBSYS_STATE(flags, shift) == PL_STATE_RDY)
#define PL_IS_ERR(flags, shift)     (GET_SUBSYS_STATE(flags, shift) == PL_STATE_ERR)
#define PL_IS_FLAGGED(flags, shift) (GET_SUBSYS_STATE(flags, shift) == PL_QUIT_FLAGGED)
#define PL_SET_RDY(flags, shift)     SET_SUBSYS_STATE(flags, shift, PL_STATE_RDY)
#define PL_SET_ERR(flags, shift)     SET_SUBSYS_STATE(flags, shift, PL_STATE_ERR)
#define PL_SET_FLAGGED(flags, shift) SET_SUBSYS_STATE(flags, shift, PL_QUIT_FLAGGED)
#define PL_CLEAR(flags, shift)       SET_SUBSYS_STATE(flags, shift, PL_STATE_NONE)

#define PL_SET_FLAG(flags, bit)    ((flags)  |=  (bit))
#define PL_CLEAR_FLAG(flags, bit)  ((flags)  &= ~(bit))
#define PL_IS_FLAG_SET(flags, bit) (((flags) &   (bit)) != 0)

enum {
    PL_STATE_NONE   = 0,  // 00 - uninitialized
    PL_STATE_RDY    = 1,  // 01 - ready
    PL_STATE_ERR    = 2,  // 10 - error
    PL_QUIT_FLAGGED = 3   // 11 - subsystem was shutdown gracefully
};


typedef struct PlatformStateT {
    void* internalState;
    u32   statusFlags;
} PlatformStateT;


b8   plStartup(PlatformStateT* platformState, u32 baudRate);
void plShutdown(PlatformStateT* platformState);
b8   plMessageStream(PlatformStateT* platformState);

void* plAllocMem(size_t size);
void  plFreeMem(void* block);

void* plSetMem(void* block, i32 value, size_t size);
void* plCopyMem(void* dest, void* src, size_t size);
void* plZeroMem(void* block, size_t size);

void plSleep(u16 ms);

void plSetFlag(PlatformStateT* platformState, u8 flag);
void plClearFlat(PlatformStateT* platformState, u8 flag);
void plFlagReady(PlatformStateT* platformState, u8 flag);
void plFlagError(PlatformStateT* platformState, u8 flag);

b8   plInitLogging(PlatformStateT* platformState);
void plStopLogging(PlatformStateT* platformState);

b8 plInitInput();
b8 plStopInput();

b8 plInitUSART(u32 baudRate);

void plConsoleWrite(const char* message);
void plConsoleWriteError(const char* message);