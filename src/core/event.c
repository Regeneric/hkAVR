#include <core/event.h>
#include <core/logger.h>
#include <platform/platform.h>

static EventCallbackF _sListenersCallback[MAX_EVENT_CODES][MAX_LISTENERS_PER_CODE];
static void*          _sListenersInstance[MAX_EVENT_CODES][MAX_LISTENERS_PER_CODE];
static u8             _sListenerCount[MAX_EVENT_CODES];

static EventT         _sEventQueue[EVENT_QUEUE_SIZE];
static volatile b8    _sQueueHead, _sQueueTail;

static PlatformStateT* eventPlatformState;


b8 hkInitEvent(PlatformStateT* platformState) {
    HTRACE("event.c -> hkEventInit(PlatformStateT*):b8");
    
    eventPlatformState = platformState;
    if(PL_IS_RDY(eventPlatformState->statusFlags, PL_EVENT)) {
        HDEBUG("hkInitEvent(): Events already initialized");
        return;
    }

    // Clear listeners count
    for(u8 i = 0; i < MAX_EVENT_CODES; ++i) _sListenerCount[i] = 0;
    
    // Reset queue pointers
    _sQueueHead = 0;
    _sQueueTail = 0;

    PL_SET_RDY(eventPlatformState->statusFlags, PL_EVENT);
    HDEBUG("hkInitEvent(): Events initialized");
    return TRUE;
}

b8 hkEventRegister(b8 code, void* listener, EventCallbackF callback) {
    HTRACE("event.c -> hkEventRegister(b8, void*, EventCallbackF):b8");
    PL_CLEAR_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);
    PL_CLEAR(eventPlatformState->statusFlags, PL_EVENT);

    if(code < 0 || code >= MAX_EVENT_CODES) {
        PL_SET_ERR(eventPlatformState->statusFlags, PL_EVENT);
        HDEBUG("hkEventRegister(): Event code unknown: %u.", code);
        return FALSE;
    }

    u8 count = _sListenerCount[code];   // Listeners of a given kind
    for(u8 i = 0; i < count; ++i) {
        // Check for duplicates
        if(_sListenersInstance[code] == listener && _sListenersCallback[code][i] == callback) {
            PL_SET_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);
            PL_SET_ERR(eventPlatformState->statusFlags , PL_EVENT);
            HDEBUG("hkEventRegister(): Found a duplicate listener");
            return FALSE;
        }
    }

    if(count >= MAX_EVENT_CODES) {
        // No space for a new listener
        PL_SET_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);
        PL_SET_ERR(eventPlatformState->statusFlags , PL_EVENT);
        HDEBUG("hkEventRegister(): No more space for new listeners");
        return FALSE;
    }

    // Append new listener to the lsit
    _sListenersInstance[code][count] = listener;
    _sListenersCallback[code][count] = callback;
    _sListenerCount[code] = count+1;

    PL_SET_RDY(eventPlatformState->statusFlags, PL_EVENT);
    HDEBUG("hkEventRegister(): Event registered | %u", code);
    return TRUE;
}

b8 hkEventUnregister(b8 code, void* listener, EventCallbackF callback) {
    HTRACE("event.c -> hkEventUnregister(b8, void*, EventCallbackF):b8");
    PL_CLEAR_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);
    PL_CLEAR(eventPlatformState->statusFlags, PL_EVENT);

    if(code < 0 || code >= MAX_EVENT_CODES) {
        PL_SET_ERR(eventPlatformState->statusFlags, PL_EVENT);
        HDEBUG("hkEventUnregister(): Event code unknown: %u.", code);
        return FALSE;
    }

    u8 count = _sListenerCount[code];   // Listeners of a given kind
    for(u8 i = 0; i < count; ++i) {
        if(_sListenersInstance[code] == listener && _sListenersCallback[code][i] == callback) {
            for(u8 j = 0; j+1 < count; ++j) {
                // Shift events back one cell - 'unregister'
                _sListenersInstance[code][j] = _sListenersInstance[code][j+1];
                _sListenersCallback[code][j] = _sListenersCallback[code][j+1];
            } _sListenerCount[code] = count-1;

            PL_SET_RDY(eventPlatformState->statusFlags, PL_EVENT);
            HDEBUG("hkEventUnregister(): Event unregistered | %u", code);
            return TRUE;
        }
    }

    PL_SET_ERR(eventPlatformState->statusFlags , PL_EVENT); 
    HDEBUG("hkEventUnregister(): Event not found | %u", code);
    return FALSE;
}

b8 hkEventFire(const EventT* event) { 
    HTRACE("event.c -> hkEventFire(const EventT*):b8");
    PL_CLEAR_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);
    PL_CLEAR(eventPlatformState->statusFlags, PL_EVENT);

    u8 next = (_sQueueHead+1) & (EVENT_QUEUE_SIZE-1);
    if(next == _sQueueTail) {
        // Queue is full, drop oldest event
        PL_SET_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);
        HDEBUG("hkEventFire(): Event queue is full; dropping the oldest event");
        _sQueueTail = (_sQueueTail+1) & (EVENT_QUEUE_SIZE-1);
    }

    PL_SET_RDY(eventPlatformState->statusFlags, PL_EVENT);
    _sEventQueue[_sQueueHead] = *event;
    _sQueueHead = next;
    return TRUE;
}

b8 hkEventPoll(EventT* event) {
    HTRACE("event.c -> hkEventPoll(EventT*):b8");
    PL_CLEAR_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);
    PL_CLEAR(eventPlatformState->statusFlags, PL_EVENT);

    if(_sQueueHead == _sQueueTail) {
        PL_SET_FLAG(eventPlatformState->statusFlags, PL_GENERAL_ERROR);
        HDEBUG("hkEventPoll(): Event buffer is empty");
        return FALSE;
    }

    *event = _sEventQueue[_sQueueTail];
    _sQueueTail = (_sQueueTail+1) & (EVENT_QUEUE_SIZE-1);
    
    PL_SET_RDY(eventPlatformState->statusFlags, PL_EVENT);
    return TRUE;
}

void hkEventProcess(void) {
    HTRACE("event.c -> hkEventProcess(void):void");
    
    EventT event;
    while(hkEventPoll(&event)) {
        u8 code  = event.code;
        u8 count = _sListenerCount[code];

        for(u8 i = 0; i < count; ++i) {
            void*          listener = _sListenersInstance[code][i];
            EventCallbackF callback = _sListenersCallback[code][i];
            
            // If this listener handles it, stop propagating
            if(callback(&event, listener)) {
                HDEBUG("hkEventProcess(): Event is being handled");
                break;
            }
        }
    }
}