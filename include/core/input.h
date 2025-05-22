#pragma once
#include <defines.h>

#include <core/event.h>

typedef struct InputLayoutT {
    PORT_t* port;
    u8 id;
    u8 pinMask;
    u8 isc;    
} InputLayoutT;

#define X(name,pin) BTN_##name,
    typedef enum {
        HK_BUTTON_LIST
        BTN_COUNT
    } ButtonLayoutT;
#undef X

#define HK_DEFINE_INPUT(port, name, pin, isc) {&(port), BTN_##name, (pin), (isc)}

b8   hkInitInput(PlatformStateT* platformState, InputLayoutT* input);
void hkStopInput();

// Usage:
// hkInputConfig(&PORTC, PIN4_bm | PIN6_bm, PORT_ISC_BOTHEDGES_gc);
HAPI void  hkInputConfig(InputLayoutT* input);