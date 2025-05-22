#if HPLATFORM_ARM_RP2350

#include <stdio.h>
#include <stdlib.h>

#include <pico/stdlib.h>
#include <hardware/gpio.h>

#include <hkmalloc.h>

#include <core/logger.h>
#include <core/event.h>
#include <core/hkmemory.h>
#include <core/input.h>
#include <platform/platform.h>

#if HK_ALLOW_MALLOC
    #warning WARNING: Using malloc() on AVR is highly discouraged!
#endif

typedef struct InternalStateT {
    u32  baudRate;
    FILE outStream;
} InternalStateT;


// ****************************************************************************
// PLATFORM SPECIFIC FUNCTIONS
// ----------------------------------------------------------------------------

b8 plStartup(PlatformStateT *platformState, u32 baudRate) {    
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

    if(!_hkInitTimer0()) {
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

    // sei();
    HINFO("Hardware IRQs enabled.");
    
    HINFO("Platform initialization successfull.");
    PL_SET_FLAG(platformState->statusFlags, PL_ALL_INIT_OK);
    return TRUE;
}



#endif