#if HPLATFORM_AVR_4809

#include <stdio.h>
#include <stdlib.h>

#include <avr/interrupt.h>

#include <hkmalloc.h>

#include <core/logger.h>
#include <core/event.h>
#include <core/hkmemory.h>
#include <core/input.h>
#include <platform/platform.h>


#if HK_ALLOW_MALLOC
    #warning WARNING: Using malloc() on AVR is highly discouraged!
#endif

#ifndef F_CPU
    #define F_CPU 20000000UL
    #warning WARNING: F_CPU defined for 20 MHz clock
#endif

#if HK_TIMER_SLEEP
    #include <avr/sleep.h>
#else
    #include <util/delay.h>
#endif

#define BAUD_REG_VAL(baud) ((u16)((F_CPU*64UL/(16UL*baud)) + 0.5))
// #define BAUD_REG_VAL(baud) ((u16)(((f32)F_CPU*64.0f/(16.0f*(f32)(baud))) + 0.5f))


typedef struct InternalStateT {
    u32  baudRate;
    FILE outStream;
} InternalStateT;


// ****************************************************************************
// HELPER FUNCTIONS
// ----------------------------------------------------------------------------
void hkDeepSleep() {
    VREF.CTRLA = 0;
    SLPCTRL.CTRLA = SLEEP_MODE_PWR_DOWN | SLPCTRL_SEN_bm;
    __asm__ __volatile__("sleep"); 
}

b8 hkInitTimer0(void) {
    HTRACE("platform_avr_4809.c -> hkInitTimer0(void):void");

    TCB0.CTRLA    = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm;
    TCB0.CCMP     = (F_CPU/2/1000) - 1;
    TCB0.INTCTRL  = TCB_CAPT_bm;
    TCB0.INTFLAGS = TCB_CAPT_bm;
    
    sei();
    return TRUE;
}

b8 hkStopTimer0(void) {
    HTRACE("platform_avr_4809.c -> hkStopTimer0(void):void"); 
    HDEBUG("hkStopTimer0(): Stopping Timer0.");

    TCB0.CTRLA    = 0;
    TCB0.CCMP     = 0;
    TCB0.INTCTRL  = 0;
    TCB0.INTFLAGS = 0;

    HINFO("Timer0 has been stopped.");
    return TRUE;
}

volatile u32 gsMillis = 0;
ISR(TCB0_INT_vect) {
    gsMillis++;
    TCB0.INTFLAGS = TCB_CAPT_bm;
}


void hkTransmitUSART(u8 data) {
    FLAG_TX;
    if(!(USART3.CTRLB & USART_TXEN_bm)) {
        // USART3 TX not enabled
        return;
    }

    u16 timeout = 10000;
    while(!(USART3.STATUS & USART_DREIF_bm)) {
        if(--timeout == 0) {
            // USART3 transmit buffer not ready (timeout)
            return;
        }
    } USART3.TXDATAL = data;
    _FLAG_TX;
}

int hkPrintUSART(char c, FILE* stream) {
    #if HK_USE_CRLF
        if(c == '\n') hkTransmitUSART((u8)'\r');
        hkTransmitUSART((u8)c);
    #else
        hkTransmitUSART((u8)c);
    #endif

    return 0;
}


// ****************************************************************************
// PLATFORM SPECIFIC FUNCTIONS
// ----------------------------------------------------------------------------

b8 plStartup(PlatformStateT *platformState, u32 baudRate) {
    REGISTER_DEBUG();
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0 << CLKCTRL_PEN_bp);     // Disable clock divider
    
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

    if(!hkInitTimer0()) {
        // millis() timer failed to init
        HERROR("plStartup(): Timer0 failed to initialize.");
        PL_SET_ERR(platformState->statusFlags, PL_TIMER);
        HARDWARE_DEBUG(platformState->statusFlags);
    } else PL_SET_RDY(platformState->statusFlags, PL_TIMER);
    HINFO("Timer0 initialized.");

    if(!hkInitEvent(platformState)) {
        HFATAL("plStartup(): Event subsystem failed to initialize.");
        PL_SET_ERR(platformState->statusFlags, PL_EVENT);
        HARDWARE_DEBUG(platformState->statusFlags);
    } else PL_SET_RDY(platformState->statusFlags, PL_EVENT);
    HINFO("Event subsystem initialized.");

    // TODO: It should be up to user if he wants any input; 
    // TODO: Take it out of plStartup()
    // if(!hkInitInput(platformState)) {
    //     HERROR("plStartup(): Input subsystem failed to initialize.");
    //     PL_SET_ERR(platformState->statusFlags, PL_INPUT);
    //     HARDWARE_DEBUG(platformState->statusFlags);
    // } else PL_SET_RDY(platformState->statusFlags, PL_INPUT);
    // HINFO("Input subsystem initialized.");

    sei();
    HINFO("Hardware IRQs enabled.");
    
    HINFO("Platform initialization successfull.");
    PL_SET_FLAG(platformState->statusFlags, PL_ALL_INIT_OK);
    return TRUE;
}

void plShutdown(PlatformStateT* platformState) {
    HTRACE("platform_avr_4809.c -> plStop(PlatformStateT*):void");

    platformState->statusFlags = (u32)0xFFFF;
    InternalStateT* internalState = (InternalStateT*)platformState->internalState;
    internalState->baudRate  = 0;

    cli();
    hkStopInput();
    hkStopEvent();
    hkStopTimer0(); PL_SET_FLAGGED(platformState->statusFlags, PL_TIMER);
    hkStopMemory();
    hkStopLogging();

    hkDeepSleep();
}

b8 plMessageStream(PlatformStateT* platformState) {
    // HTRACE("platform_avr_4809.c -> plMessageStream(PlatformStateT*):b8");    // I do not recommend uncommenting this line

    InternalStateT* internalState = (InternalStateT*)platformState->internalState;
    if(PL_IS_RDY(platformState->statusFlags, PL_EVENT) && PL_IS_FLAG_SET(platformState->statusFlags, PL_ALL_INIT_OK)) {
        hkEventProcess();
    } return TRUE;
}


// ----------------------------------------------------------------------------
// 

void* plAllocMem(u16 size) {
    HTRACE("platform_avr_4809.c -> plAllocMem(u16):void*");
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

void plFreeMem(void* block) {
    HTRACE("platform_avr_4809.c -> plFreeMem(void*):void");
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

void* plZeroMem(void* block, u16 size) {
    HTRACE("platform_avr_4809.c -> plZeroMem(void*, u16)");
    plSetMem(block, 0, size);
}

void* plCopyMem(void* dest, void* source, u16 size) {
    HTRACE("platform_avr_4809.c -> plCopyMem(void*, void*, u16):void");

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


void plSleep(u16 ms) {
    HTRACE("platform_avr_4809.c -> plSleep(u16):void");
    HDEBUG("HK_TIMER_SLEEP: %s", HK_TIMER_SLEEP ? "TRUE" : "FALSE");

    #if HK_TIMER_SLEEP
        if(ms == 0) {
            HWARN("plSleep(): Cannot sleep for %u ms.", ms);
            return;
        }

        u32 end = gsMillis + ms;
        while((i32)(end - gsMillis) > 0) {
            set_sleep_mode(SLEEP_MODE_IDLE);
            sleep_enable();
            sei();          
            sleep_cpu();    // wakes on the next 1 ms tick ISR
            sleep_disable();
        }
    #else        
        _delay_ms(ms);
    #endif 
}


void plSetFlag(PlatformStateT* platformState, u8 flag) {
    PL_SET_FLAG(platformState->statusFlags, flag);
}
void plClearFlag(PlatformStateT* platformState, u8 flag) {
    PL_CLEAR_FLAG(platformState->statusFlags, flag);
}
void plFlagReady(PlatformStateT* platformState, u8 flag) {
    PL_SET_RDY(platformState->statusFlags, flag);
}
void plFlagClear(PlatformStateT* platformState, u8 flag) {
    PL_SET_ERR(platformState->statusFlags, flag);
}


b8 plInitUSART(u32 baudRate) {
    // Absolute minimum speed while using 20 MHz clock
    if(baudRate <= 1220) return FALSE;

    PORTMUX.USARTROUTEA |= PORTMUX_USART3_ALT1_gc;  // Use alternatives PB4 and PB5 for USART
    PORTB.DIRSET = PIN4_bm; // PB4 as TX
    PORTB.DIRCLR = PIN5_bm; // PB5 as RX

    // Reset sequence
    USART3.CTRLA = 0;
    USART3.CTRLB = 0;

    USART3.BAUD  = BAUD_REG_VAL(baudRate);          // Calculate baud rate
    USART3.CTRLB = USART_TXEN_bm | USART_RXEN_bm;   // Enable transmitter and reciever
    USART3.CTRLC = USART_CHSIZE_8BIT_gc;            // 8N1

    if(!(USART3.CTRLB & USART_TXEN_bm)) return FALSE;

    HTRACE("platform_avr_4809.c -> plInitUSART(u32):b8");
    HDEBUG("plInitUSART(): USART initialized on PB4 and PB5");
    return TRUE;
}

b8 plInitLogging(PlatformStateT* platformState) {
    InternalStateT* internalState = (InternalStateT*)platformState->internalState;

    if(!plInitUSART(internalState->baudRate)) {
        PL_SET_ERR(platformState->statusFlags, PL_USART);
        HARDWARE_DEBUG(platformState->statusFlags);
        return FALSE;
    }

    fdev_setup_stream(&internalState->outStream, hkPrintUSART, NULL, _FDEV_SETUP_WRITE);
    stdout = &internalState->outStream;
    stderr = &internalState->outStream;

    PL_SET_RDY(platformState->statusFlags, PL_USART);
    return TRUE;
}

void plStopLogging(PlatformStateT* platformState) {
    HTRACE("platform_avr_4809.c -> plStopLogging(PlatformStateT*):void");
    HDEBUG("plStopLogging(): Stopping logging...");

    PL_SET_FLAGGED(platformState->statusFlags, PL_USART);
    PL_SET_FLAGGED(platformState->statusFlags, PL_LOGGING);

    PORTB.DIRCLR = 0xFF;

    USART3.CTRLA = 0;
    USART3.CTRLB = 0;
    USART3.CTRLC = 0;

    return;
}


void plConsoleWrite(const char* message) {
    if(message == NULL) return;
    printf("%s", message);
}
void plConsoleWriteError(const char* message) {
    if(message == NULL) return;
    fprintf(stderr, "%s", message);
}





// ISR(PORTA_PORT_vect) {
//     u8 flags       = PORTA.INTFLAGS;
//     PORTA.INTFLAGS = flags;

//     for(u8 i = 0; i != BTN_COUNT; ++i) {
//         const InputLayoutT *btn = &_sgInputButtons[i];
//         if(flags & btn->pinMask) {            
//             b8 pressed = !(btn->port->IN & btn->pinMask);

//             EventT event;
//             event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
//             event.sender  = NULL;    // Or &whateverHere
//             event.data[0] = btn->id;
//             event.data[1] = pressed;

//             hkEventFire(&event);
//         }
//     }
// }

// ISR(PORTB_PORT_vect) {
//     u8 flags       = PORTB.INTFLAGS;
//     PORTB.INTFLAGS = flags;

//     for(u8 i = 0; i != BTN_COUNT; ++i) {
//         const InputLayoutT *btn = &_sgInputButtons[i];
//         if(flags & btn->pinMask) {            
//             b8 pressed = !(btn->port->IN & btn->pinMask);

//             EventT event;
//             event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
//             event.sender  = NULL;    // Or &whateverHere
//             event.data[0] = btn->id;
//             event.data[1] = pressed;

//             hkEventFire(&event);
//         }
//     }
// }

// ISR(PORTC_PORT_vect) {
//     u8 flags       = PORTC.INTFLAGS;
//     PORTC.INTFLAGS = flags;

//     for(u8 i = 0; i != BTN_COUNT; ++i) {
//         const InputLayoutT *btn = &_sgInputButtons[i];
//         if(flags & btn->pinMask) {            
//             b8 pressed = !(btn->port->IN & btn->pinMask);

//             EventT event;
//             event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
//             event.sender  = NULL;    // Or &whateverHere
//             event.data[0] = btn->id;
//             event.data[1] = pressed;

//             hkEventFire(&event);
//         }
//     }
// }

// ISR(PORTC_PORT_vect) {
//     u8 flags       = PORTC.INTFLAGS;
//     PORTC.INTFLAGS = flags;

//     for(u8 i = 0; i != BTN_COUNT; ++i) {
//         const InputLayoutT *btn = &_sgInputButtons[i];
//         if(flags & btn->pinMask) {            
//             b8 pressed = !(btn->port->IN & btn->pinMask);

//             EventT event;
//             event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
//             event.sender  = NULL;    // Or &whateverHere
//             event.data[0] = btn->id;
//             event.data[1] = pressed;

//             hkEventFire(&event);
//         }
//     }
// }

// ISR(PORTD_PORT_vect) {
//     u8 flags       = PORTD.INTFLAGS;
//     PORTD.INTFLAGS = flags;

//     for(u8 i = 0; i != BTN_COUNT; ++i) {
//         const InputLayoutT *btn = &_sgInputButtons[i];
//         if(flags & btn->pinMask) {            
//             b8 pressed = !(btn->port->IN & btn->pinMask);

//             EventT event;
//             event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
//             event.sender  = NULL;    // Or &whateverHere
//             event.data[0] = btn->id;
//             event.data[1] = pressed;

//             hkEventFire(&event);
//         }
//     }
// }

// ISR(PORTE_PORT_vect) {
//     u8 flags       = PORTE.INTFLAGS;
//     PORTE.INTFLAGS = flags;

//     for(u8 i = 0; i != BTN_COUNT; ++i) {
//         const InputLayoutT *btn = &_sgInputButtons[i];
//         if(flags & btn->pinMask) {            
//             b8 pressed = !(btn->port->IN & btn->pinMask);

//             EventT event;
//             event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
//             event.sender  = NULL;    // Or &whateverHere
//             event.data[0] = btn->id;
//             event.data[1] = pressed;

//             hkEventFire(&event);
//         }
//     }
// }

// ISR(PORTF_PORT_vect) {
//     u8 flags       = PORTF.INTFLAGS;
//     PORTF.INTFLAGS = flags;

//     for(u8 i = 0; i != BTN_COUNT; ++i) {
//         const InputLayoutT *btn = &_sgInputButtons[i];
//         if(flags & btn->pinMask) {            
//             b8 pressed = !(btn->port->IN & btn->pinMask);

//             EventT event;
//             event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
//             event.sender  = NULL;    // Or &whateverHere
//             event.data[0] = btn->id;
//             event.data[1] = pressed;

//             hkEventFire(&event);
//         }
//     }
// }
#endif