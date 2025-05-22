#include <defines.h>

#include <core/logger.h>
#include <core/event.h>
#include <core/input.h>
#include <platform/platform.h>

#include <containers/darray.h>

b8 hkOnEvent(const EventT* event, void* listener);
b8 hkOnButton(const EventT* event, void* listener);

static PlatformStateT platformState;

int main() {
    plStartup(&platformState, HK_BAUD_RATE);

    static const InputLayoutT buttons[BTN_COUNT] = {
        {&HK_INPUT_REG, BTN_ACCEPT, HK_ACCEPT_BTN, PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm},
        {&HK_INPUT_REG, BTN_CANCEL, HK_CANCEL_BTN, PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm}
    };

    if(!hkInitInput(&platformState, &buttons)) {
        HERROR("plStartup(): Input subsystem failed to initialize.");
    } else HINFO("Input subsystem initialized.");

    // u8* test = (u8*)plAllocMem(1 + sizeof(u8));
    // plFreeMem(test);
    
    // u8* test2 = hkDarrayCreate(u8);
    // hkDarrayPush(test2, 10);
    // hkDarrayPush(test2, 20);
    // for(u8 i = 0; i != hkDarrayLength(test2); ++i) HDEBUG("test[%u]: %u", i, test2[i]);

    hkEventRegister(0x69            , 0, hkOnEvent);
    hkEventRegister(EC_PLATFORM_STOP, 0, hkOnEvent);
    hkEventRegister(EC_BTN_PRESSED  , 0, hkOnButton);
    hkEventRegister(EC_BTN_RELEASED , 0, hkOnButton);

    while(PL_IS_FLAG_SET(platformState.statusFlags, PL_ALL_INIT_OK)) {
        // HDEBUG("test");
        
        // if(PL_IS_FLAG_SET(platformState.statusFlags, PL_MALLOC_WARN)) {
        //     HDEBUG("malloc() is on");
        // } else HDEBUG("malloc() is off");

        // if(PL_IS_RDY(platformState.statusFlags, PL_LOGGING)) {
        //     HDEBUG("Logger is on");
        // } else HDEBUG("Logger is off");

        // if(PL_IS_RDY(platformState.statusFlags, PL_EVENT)) {
        //     HDEBUG("Events are on");
        // } else HDEBUG("Events are off");

        // if(PL_IS_RDY(platformState.statusFlags, PL_MEMORY)) {
        //     HDEBUG("Memory is on");
        // } else HDEBUG("Memory is off");
        
        plMessageStream(&platformState);
        // plSleep(1000);
    } return 0;
}


b8 hkOnEvent(const EventT* event, void* listener) {
    HTRACE("main.c -> hkOnEvent(const EventT*, void*):b8");

    switch(event->code) {
        case EC_PLATFORM_STOP: {
            HINFO("EC_PLATFORM_STOP event recieved; shutting down...");
            plShutdown(&platformState);
            return TRUE;
        }
    } return TRUE;
}

b8 hkOnButton(const EventT* event, void* listener) {
    HTRACE("main.c -> hkOnButton(const EventT*, void*):b8");

    u8 buttonID    = (u8)event->data[0];
    b8 buttonState = (b8)event->data[1];

    switch(event->code) {
        case EC_BTN_PRESSED: {
            HDEBUG("hkOnButton(): Button %u %s", buttonID, buttonState ? "pressed" : "released");
            switch(buttonID) {
                case BTN_ACCEPT: {
                    EventT e;
                    e.code    = EC_PLATFORM_STOP;
                    e.data[0] = 0;
                    e.data[1] = 0;
                    e.sender  = NULL;
                
                    hkEventFire(&e);
                } default: return TRUE;
            }
        } default: return TRUE;
    } return TRUE;
}

// b8 hkOnButton(const EventT* event, void* listener) {
//     HTRACE("main.c -> hkOnButton(const EventT*, void*):b8");

//     u8 buttonID    = (u8)event->data[0];
//     b8 buttonState = (b8)event->data[1];

//     switch(event->code) {
//         case EC_BTN_PRESSED: {
//             HDEBUG("hkOnButton(): Button %u %s", buttonID, buttonState ? "pressed" : "released");

//             if(buttonID == BTN_ACCEPT) {
//                 EventT e;
//                 e.code    = EC_PLATFORM_STOP;
//                 e.data[0] = 0;
//                 e.data[1] = 0;
//                 e.sender  = NULL;
            
//                 hkEventFire(&e);
//             }
//         } return TRUE;
//     } return TRUE;
// }