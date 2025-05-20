#include <defines.h>

#include <core/logger.h>
#include <platform/platform.h>


// #include <avr/io.h>
#include <util/delay.h>

int main() {
    PlatformStateT platformState;
    plStartup(&platformState, BAUD_RATE);

    u8* test = (u8*)plAllocMem(1 + sizeof(u8));
    plFreeMem(test);
    
    while(PL_IS_FLAG_SET(platformState.statusFlags, PL_ALL_INIT_OK)) {
        plSleep(1000);
        HDEBUG("test");
        
        if(PL_IS_FLAG_SET(platformState.statusFlags, PL_MALLOC_WARN)) {
            HDEBUG("malloc() is on");
        } else HDEBUG("malloc() is off");

        if(PL_IS_RDY(platformState.statusFlags, PL_LOGGING)) {
            HDEBUG("logger is on");
        } else HDEBUG("logger is off");
    }

    return 0;
}