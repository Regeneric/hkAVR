#include <core/event.h>
#include <core/logger.h>
#include <platform/platform.h>

#include <containers/darray.h>

// static EventCallbackF _sListenersCallback[MAX_EVENT_CODES][MAX_LISTENERS_PER_CODE];
// static void*          _sListenersInstance[MAX_EVENT_CODES][MAX_LISTENERS_PER_CODE];
// static u8             _sListenerCount[MAX_EVENT_CODES];

static EventListenerT* _sListeners[MAX_EVENT_CODES];
static EventT* _sEventQueue;

// static EventT         _sEventQueue[EVENT_QUEUE_SIZE];
// static volatile b8    _sQueueHead, _sQueueTail;

static PlatformStateT* eventPlatformState;


b8 hkInitEvent(void* platformState) {
    HTRACE("event.c -> hkEventInit(PlatformStateT*):b8");
    eventPlatformState = (PlatformStateT*)platformState;

    if(PL_IS_RDY(eventPlatformState->statusFlags, PL_EVENT)) {
        HDEBUG("hkInitEvent(): Events already initialized");
        return FALSE;
    }

    // Clear listeners count
    for(u8 i = 0; i < MAX_EVENT_CODES; ++i) {
        // _sListenerCount[i] = 0;
        _sListeners[i] = hkDarrayCreate(EventListenerT);
        if(!_sListeners[i]) {
            HERROR("hkInitEvent(): Out of Memory for _sListeners!");
            return FALSE;
        }
    }

    _sEventQueue = hkDarrayCreate(EventT);
    if(!_sEventQueue) {
        HERROR("hkInitEvent(): Out of Memory for _sEventQueue!");
        return FALSE;
    }

    PL_SET_RDY(eventPlatformState->statusFlags, PL_EVENT);
    HDEBUG("hkInitEvent(): Events initialized");
    return TRUE;
}

void hkStopEvent() {
    HTRACE("event.c -> hkStopEvent():void");
    HDEBUG("hkStopEvent(): Stopping event subsystem.");
    
    for(u8 i = 0; i != MAX_EVENT_CODES; ++i) hkDarrayDestroy(_sListeners[i]);
    hkDarrayDestroy(_sEventQueue);

    PL_SET_FLAGGED(eventPlatformState->statusFlags, PL_INPUT);
    HINFO("Event subsytem has been stopped.");
}


b8 hkEventRegister(u16 code, void* listener, EventCallbackF callback) {
    HTRACE("event.c -> hkEventRegister(b8, void*, EventCallbackF):b8");
    PL_CLEAR_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);

    if(code >= EC_MAX_EVENT_CODE) {
        PL_SET_ERR(eventPlatformState->statusFlags, PL_EVENT);
        HDEBUG("hkEventRegister(): Event code unknown: 0x%x.", code);
        return FALSE;
    }

    // Lazy‐allocate the listener list for this code
    if (!_sListeners[code]) {
        _sListeners[code] = hkDarrayCreate(EventListenerT);
        if (!_sListeners[code]) {
            HERROR("hkEventRegister(): Out of Memory creating listener list for 0x%x", code);
            PL_SET_ERR(eventPlatformState->statusFlags, PL_EVENT);
            return FALSE;
        }
    }

    u16 count = hkDarrayLength(_sListeners[code]);
    for(u16 i = 0; i < count; ++i) {
        EventListenerT* L = &_sListeners[code][i];
        if(L->listener == listener && L->callback == callback) {
            PL_SET_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);
            PL_SET_ERR(eventPlatformState->statusFlags , PL_EVENT);
            HDEBUG("hkEventRegister(): Found a duplicate listener for event 0x%x");
            return FALSE;
        }
    }

    // Append new listener to the lsit
    EventListenerT lt = {listener, callback};
    hkDarrayPush(_sListeners[code], lt);

    PL_SET_RDY(eventPlatformState->statusFlags, PL_EVENT);
    HDEBUG("hkEventRegister(): Event registered | 0x%x", code);
    return TRUE;
}

b8 hkEventUnregister(u16 code, void* listener, EventCallbackF callback) {
    HTRACE("event.c -> hkEventUnregister(b8, void*, EventCallbackF):b8");
    PL_CLEAR_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);

    if(code >= EC_MAX_EVENT_CODE) {
        PL_SET_ERR(eventPlatformState->statusFlags, PL_EVENT);
        HDEBUG("hkEventUnregister(): Event code unknown: 0x%x.", code);
        return FALSE;
    }

    u16 count = hkDarrayLength(_sListeners[code]);
    for(u16 i = 0; i < count; ++i) {
        EventListenerT* L = &_sListeners[code][i];
        if(L->listener == listener && L->callback == callback) {
            _sListeners[code] = hkDarrayPopAt(_sListeners[code], i, L);
            
            PL_SET_RDY(eventPlatformState->statusFlags, PL_EVENT);
            HDEBUG("hkEventUnregister(): Event unregistered | 0x%x", code);
            return TRUE;
        }
    }

    PL_SET_ERR(eventPlatformState->statusFlags , PL_EVENT); 
    HDEBUG("hkEventUnregister(): Event not found | 0x%x", code);
    return FALSE;
}

b8 hkEventFire(const EventT* event) { 
    HTRACE("event.c -> hkEventFire(const EventT*):b8");
    PL_CLEAR_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);

    hkDarrayPush(_sEventQueue, *event);

    PL_SET_RDY(eventPlatformState->statusFlags, PL_EVENT);
    return TRUE;
}

b8 hkEventPoll(EventT* event) {
    // HTRACE("event.c -> hkEventPoll(EventT*):b8");    // I do not recommend uncommenting this line
    PL_CLEAR_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);

    u16 count = hkDarrayLength(_sEventQueue);
    if(count == 0) {
        PL_SET_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);
        return FALSE;
    } 

    _sEventQueue = hkDarrayPopAt(_sEventQueue, 0, event);
    if(hkDarrayLength(_sEventQueue) > 0) HDEBUG("hkEventPoll(): Polling event: 0x%x", event->code);

    PL_SET_RDY(eventPlatformState->statusFlags, PL_EVENT);
    return TRUE;
}

void hkEventProcess(void) {
    // HTRACE("event.c -> hkEventProcess(void):void");  // I do not recommend uncommenting this line
    
    static EventT event;
    while(hkEventPoll(&event)) {
        u16 code  = event.code;
        u16 count = hkDarrayLength(_sListeners[code]);

        for(u16 i = 0; i < count; ++i) {
            EventListenerT* eventListener = &_sListeners[code][i];
            if(eventListener->callback(&event, eventListener->listener)) {
                HDEBUG("hkEventProcess(): Event 0x%x is being handled", code);
                break;
            }
        }
    }
}