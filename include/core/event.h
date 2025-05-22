#pragma once
#include <defines.h>

#define MAX_EVENT_CODES  (EC_MAX_EVENT_CODE + 1)

typedef struct EventT {  
    u16   code;       // Event code
    u16   data[2];    // Payload
    void* sender;     // Callback, can be 0 or NULL
} EventT;

typedef b8 (*EventCallbackF)(const EventT* event, void* listener);

typedef struct EventListenerT {
    void*          listener;
    EventCallbackF callback;
} EventListenerT;

typedef enum EventCodeT {
    EC_BTN_PRESSED    = 0x00,
    EC_BTN_RELEASED   = 0x01,
    
    EC_PLATFORM_STOP  = 0x1F,
    EC_MAX_EVENT_CODE = 0x20
} EventCodeT;


b8 hkEventPoll(EventT* event);
HAPI void hkEventProcess(void);

HAPI b8   hkInitEvent(void* platformState);
HAPI void hkStopEvent();

HAPI b8 hkEventRegister(u16 code, void* listener, EventCallbackF callback);
HAPI b8 hkEventUnregister(u16 code, void* listener, EventCallbackF callback);

HAPI b8 hkEventFire(const EventT* event);