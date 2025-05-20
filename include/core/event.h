#pragma once
#include <defines.h>

#include <platform/platform.h>

#define MAX_EVENT_CODES         4   // How many distinct events can be registered 
#define MAX_LISTENERS_PER_CODE  2   // Max listeners per event type
#define EVENT_QUEUE_SIZE        8   // Max events hold in the buffer

typedef struct EventT {  
    u8    code;       // Event code
    u16   data[2];    // Payload
    void *sender;     // Callback, can be 0 or NULL
} EventT;

typedef b8 (*EventCallbackF)(const EventT* event, void* listener);

b8 hkInitEvent(PlatformStateT* platformState);

b8 hkEventRegister(b8 code, void* listener, EventCallbackF callback);
b8 hkEventUnregister(b8 code, void* listener, EventCallbackF callback);

b8 hkEventFire(const EventT* event);
b8 hkEventPoll(EventT* event);