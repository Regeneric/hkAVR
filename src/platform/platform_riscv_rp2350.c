#include <defines.h>
#ifdef HPLATFORM_RISCV_2350

#include <stdio.h>
#include <stdlib.h>

#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <pico/stdio.h>
#include <pico/stdio_usb.h>
#include <pico/stdio_uart.h>
#include <pico/stdio/driver.h>

#include <hkmalloc.h>

#include <core/logger.h>
#include <core/event.h>
#include <core/hkmemory.h>
#include <core/input.h>
#include <platform/platform.h>


typedef struct InternalStateT {
    u32  baudRate;
    FILE outStream;
} InternalStateT;

static InputLayoutT* _sgInputButtons;

#define MAX_GPIO 30


// ****************************************************************************
// HELPER FUNCTIONS
// ----------------------------------------------------------------------------
static inline void _hkDisableGPIO(InputLayoutT* input) {
    HTRACE("platform_riscv_2359.c -> _hkDisablePortInput(InputLayoutT*):void");

    for(u8 gpio = 0; gpio != MAX_GPIO; ++gpio) {
        if(input->pinMask == gpio) gpio_deinit(gpio);
    }
}

void _hkInputCallback(uint gpio, uint32_t events) {
    HTRACE("platform_riscv_2350.c -> _hkInputCallback(uint, uint32_t):void");

    for(u8 i = 0; i != BTN_COUNT; ++i) {
        const InputLayoutT *btn = &_sgInputButtons[i];

        if(gpio == btn->pinMask) {
            b8 pressed = gpio_get(gpio) == 0;

            EventT event;
            event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
            event.sender  = NULL;    // Or &whateverHere
            event.data[0] = btn->id;
            event.data[1] = pressed;

            hkEventFire(&event);
        }
    }
}

void _hkInputConfig(PlatformStateT* platformState, InputLayoutT* input) {
    HTRACE("platform_riscv_2350.c -> _hkInputConfig(PlatformStateT*, InputLayoutT*):void");

    if(!PL_IS_RDY(platformState->statusFlags, PL_INPUT)) {
        HERROR("_hkInputConfig(): Input subsystem mus be initialized first!");
        PL_SET_FLAGGED(platformState->statusFlags, PL_GENERAL_ERROR);
        return;
    }
    
    if(input->name != NULL) HDEBUG("Pin 0x%x of %s as input", input->pinMask, input->name);
    else HDEBUG("Pin 0x%x as input", input->pinMask);
 
    for(u8 gpio = 0; gpio != MAX_GPIO; ++gpio) {
        if(input->pinMask == gpio) {
            gpio_init(gpio);
            gpio_set_dir(gpio, GPIO_IN);
            gpio_pull_up(gpio);
            gpio_set_irq_enabled_with_callback(gpio, input->isc, TRUE, &_hkInputCallback);
        }
    }
    
    return;
}

void _hkDeepSleep() {return;}

static inline void _hkWaitForTimerIRQ() {__asm volatile ("wfi");}

// ****************************************************************************
// PLATFORM SPECIFIC FUNCTIONS
// ----------------------------------------------------------------------------

b8 plStartup(PlatformStateT* platformState, u32 baudRate) {
    stdio_init_all();

    // HTRACE("platform_riscv_2350.c -> plStartup(PlatformStateT*, u32):b8"); 

    platformState->statusFlags = (u32)0x0000;
    #if HK_ALLOW_MALLOC
        platformState->internalState = malloc(sizeof(InternalStateT));
        PL_SET_FLAG(platformState->statusFlags, PL_MALLOC_WARN);
    #else
        platformState->internalState = hkMalloc(sizeof(InternalStateT));
        PL_CLEAR_FLAG(platformState->statusFlags, PL_MALLOC_WARN);
    #endif

    InternalStateT* internalState = (InternalStateT*)platformState->internalState;
    internalState->baudRate = baudRate;

    // Subsystems
    if(!plInitLogging(platformState)) {
        // Logger subsystem failed to init
        PL_SET_ERR(platformState->statusFlags, PL_LOGGING);
        HARDWARE_DEBUG(platformState->statusFlags);
    } else PL_SET_RDY(platformState->statusFlags, PL_LOGGING);
    HINFO("Logging subsystem initialized.");

    if(!hkInitMemory(platformState)) {
        HFATAL("plStartup(): Memory subsystem failed to initialize.");
        PL_SET_ERR(platformState->statusFlags, PL_MEMORY);
        HARDWARE_DEBUG(platformState->statusFlags);
    } else PL_SET_RDY(platformState->statusFlags, PL_MEMORY);
    HINFO("Memory subsystem initialized.");

    if(!hkInitEvent(platformState)) {
        HFATAL("plStartup(): Event subsystem failed to initialize.");
        PL_SET_ERR(platformState->statusFlags, PL_EVENT);
        HARDWARE_DEBUG(platformState->statusFlags);
    } else PL_SET_RDY(platformState->statusFlags, PL_EVENT);
    HINFO("Event subsystem initialized.");

    HINFO("Platform initialization successfull.");
    PL_SET_FLAG(platformState->statusFlags, PL_ALL_INIT_OK);
    return TRUE;   
}

void plShutdown(PlatformStateT* platformState) {
    HTRACE("platform_riscv_2350.c -> plStop(PlatformStateT*):void");

    platformState->statusFlags = (u32)0xFFFF;
    InternalStateT* internalState = (InternalStateT*)platformState->internalState;
    internalState->baudRate  = 0;

    plStopInput(platformState);
    hkStopEvent();
    hkStopMemory();
    hkStopLogging();

    _hkDeepSleep();
}

b8 plMessageStream(PlatformStateT* platformState) {
    // HTRACE("platform_riscv_2350.c -> plMessageStream(PlatformStateT*):b8");    // I do not recommend uncommenting this line

    InternalStateT* internalState = (InternalStateT*)platformState->internalState;
    if(PL_IS_RDY(platformState->statusFlags, PL_EVENT) && PL_IS_FLAG_SET(platformState->statusFlags, PL_ALL_INIT_OK)) {
        hkEventProcess();
    } return TRUE;
}


// ----------------------------------------------------------------------------
// 

void* plAllocMem(u16 size) {
    HTRACE("platform_riscv_2350.c -> plAllocMem(u16):void*");
    HTRACE("plAllocMem(): HK_ALLOW_MALLOC: %s", HK_ALLOW_MALLOC ? "TRUE" : "FALSE");
    
    #if HK_ALLOW_MALLOC
        HWARN("plAllocMem(): Using malloc() on AVR is highly discouraged!");
        void* buffer = malloc((u16)size);
    #else
        HTRACE("plAllocMem(): Using hkMalloc() instead of malloc()");
        void* buffer = hkMalloc((u16)size);
    #endif

    if(buffer == NULL) {
        HFATAL("plAllocMem(): Out of Memory!");
        return NULL;
    } else return buffer;
}

void  plFreeMem(void* block) {
    HTRACE("platform_riscv_2350.c -> plFreeMem(void*):void");
    HTRACE("plFreeMem(): HK_ALLOW_MALLOC: %s", HK_ALLOW_MALLOC ? "TRUE" : "FALSE");

    #if HK_ALLOW_MALLOC
        HWARN("plFreeMem(): Using free() on AVR is highly discouraged!");
        free(block);
    #else
        HTRACE("plFreeMem(): Using hkFree() instead of free()");
        hkFree(block);
    #endif
}

void* plSetMem(void* block, i32 value, u16 size) {
    if(block == NULL) return NULL;
    if(size == 0) return NULL;

    u8* ptr = (u8*)block;
    u8* end = ptr + size;

    while(ptr < end) *ptr++ = (u8)value;
    return block;
}

void* plCopyMem(void* dest, const void* source, u16 size) {
    HTRACE("platform_riscv_2350.c -> plCopyMem(void*, void*, u16):void");

    if(dest == NULL) {
        HDEBUG("plCopyMem(): Passed `dest` is NULL");
        return NULL;
    }
    if(source == NULL) {
        HDEBUG("plCopyMem(): Passed `source` is NULL");
        return NULL;
    }
    if(size == 0) {
        HDEBUG("plCopyMem(): Passed `size` is 0");
        return NULL;
    }

    u8* srcPtr = (u8*)source;
    u8* dstPtr = (u8*)dest;
    u8* dstEnd = dstPtr + size;

    while(dstPtr < dstEnd) *dstPtr++ = *srcPtr++;
    return dest;
}

void* plZeroMem(void* block, u16 size) {
    HTRACE("platform_riscv_2350.c -> plZeroMem(void*, u16)");
    plSetMem(block, 0, size);
}


void plSleep(u16 ms) {
    HTRACE("platform_riscv_2350.c -> plSleep(u16):void");
    HDEBUG("HK_TIMER_SLEEP: %s", HK_TIMER_SLEEP ? "TRUE" : "FALSE");

    if(ms == 0) {
        HWARN("plSleep(): Cannot sleep for %u ms.", ms);
        return;
    }

    #if HK_TIMER_SLEEP
        u64 targetTime = to_ms_since_boot(get_absolute_time()) + ms;
        while(to_ms_since_boot(get_absolute_time()) < targetTime) {
            _hkWaitForTimerIRQ();
        }
    #else        
        sleep_ms((u16)ms);
    #endif 
}


void plSetFlag(PlatformStateT* platformState, u32 flag) {
    PL_SET_FLAG(platformState->statusFlags, flag);
}
void plClearFlag(PlatformStateT* platformState, u32 flag) {
    PL_CLEAR_FLAG(platformState->statusFlags, flag);
}
void plFlagReady(PlatformStateT* platformState, u32 flag) {
    PL_SET_RDY(platformState->statusFlags, flag);
}
void plFlagClear(PlatformStateT* platformState, u32 flag) {
    PL_SET_ERR(platformState->statusFlags, flag);
}


b8 plInitUSART(u32 baudRate) {
    HTRACE("platform_riscv_2350.c -> plInitUSART(u32):b8");

    // Absolute minimum speed while using 20 MHz clock
    if(baudRate <= 1220) return FALSE;

    // TODO: Allow user to change uart pins
    gpio_init(PICO_DEFAULT_UART_TX_PIN); 
    gpio_set_dir(PICO_DEFAULT_UART_TX_PIN, GPIO_OUT);

    gpio_init(PICO_DEFAULT_UART_RX_PIN);
    gpio_set_dir(PICO_DEFAULT_UART_RX_PIN, GPIO_IN);

    // Couldn't set passed baud rate
    if(uart_init(uart0, baudRate) != baudRate) return FALSE;

    gpio_set_function(PICO_DEFAULT_UART_TX_PIN, UART_FUNCSEL_NUM(uart0, PICO_DEFAULT_UART_TX_PIN));
    gpio_set_function(PICO_DEFAULT_UART_RX_PIN, UART_FUNCSEL_NUM(uart0, PICO_DEFAULT_UART_RX_PIN));

    // TODO: Allow user to change uart na data frame format
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);     // 8N1

    HTRACE("platform_riscv_2350.c -> plInitUSART(u32):b8");
    HDEBUG("plInitUSART(): USART initialized on GPIO0 and GPIO1");
    return TRUE;
}

b8 plInitLogging(PlatformStateT* platformState) {
    #if HK_LOG_OUTPUT == USB
        // stdio_filter_driver(&stdio_usb);
        HDEBUG("plInitInput(): TinyUSB support enabled.");
    #elif HK_LOG_OUTPUT == UART
        InternalStateT* internalState = (InternalStateT*)platformState->internalState;

        if(!plInitUSART(internalState->baudRate)) {
            PL_SET_ERR(platformState->statusFlags, PL_USART);
            HARDWARE_DEBUG(platformState->statusFlags);
            return FALSE;
        }

        stdio_filter_driver(&stdio_uart);
        PL_SET_RDY(platformState->statusFlags, PL_USART);
    #endif

    return TRUE;
}

void plStopLogging(PlatformStateT* platformState) {
    stdio_filter_driver(NULL);
    uart_deinit(uart0);
}


b8 plInitInput(PlatformStateT* platformState, void* platformInput) {
    HTRACE("platform_riscv_2350.c -> plInitInput(PlatformStateT*, InputLayoutT*):b8");

    InputLayoutT* input = (InputLayoutT*)platformInput; 

    PL_SET_RDY(platformState->statusFlags, PL_INPUT);
    _sgInputButtons = input;
    
    for(u8 i = 0; i != BTN_COUNT; ++i) _hkInputConfig(platformState, &input[i]);

    return TRUE;
}

void plStopInput(PlatformStateT* platformState) {
    HTRACE("platform_riscv_2350.c -> plStopInput(InputLayoutT*):void");

    _hkDisableGPIO(_sgInputButtons);

    PL_SET_FLAGGED(platformState->statusFlags, PL_INPUT);
}

void plInputConfig(PlatformStateT* platformState, void* platformInput) {
    HTRACE("platform_riscv_2350.c -> plInputConfig(InputLayoutT*):void");

    InputLayoutT* input = (InputLayoutT*)platformInput; 
    _hkInputConfig(platformState, input);
}


void plConsoleWrite(const char* message) {
    if(message == NULL) return;
    printf("%s", message);
}
void plConsoleWriteError(const char* message) {
    if(message == NULL) return;
    fprintf(stderr, "%s", message);
}


u32 plGetAbsoluteTime() {
    return to_ms_since_boot(get_absolute_time());
}
#endif