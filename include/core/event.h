#pragma once
#include <defines.h>

#include <platform/platform.h>

#define MAX_EVENT_CODES         8   // How many distinct events can be registered 
#define MAX_LISTENERS_PER_CODE  2   // Max listeners per event type
#define EVENT_QUEUE_SIZE        8   // Max events hold in the buffer

typedef struct EventListenerT {
    void*          listener;
    EventCallbackF callback;
} EventListenerT;

typedef enum EventCodeT {
    EC_BTN_PRESSED    = 0x0,
    EC_BTN_RELEASED   = 0x1,
    
    EC_PLATFORM_STOP  = 0x6,
    EC_MAX_EVENT_CODE = 0x7
} EventCodeT;

typedef struct EventT {  
    u8    code;       // Event code
    u16   data[2];    // Payload
    void *sender;     // Callback, can be 0 or NULL
} EventT;

typedef b8 (*EventCallbackF)(const EventT* event, void* listener);

b8 hkEventPoll(EventT* event);
HAPI void hkEventProcess(void);

HAPI b8   hkInitEvent(PlatformStateT* platformState);

HAPI b8 hkEventRegister(b8 code, void* listener, EventCallbackF callback);
HAPI b8 hkEventUnregister(b8 code, void* listener, EventCallbackF callback);

HAPI b8 hkEventFire(const EventT* event);