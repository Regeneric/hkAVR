#include <core/input.h>
#include <core/logger.h>
#include <platform/platform.h>

#include <avr/interrupt.h>
#include <avr/io.h>


static PlatformStateT* _sgInputPlatformState;
static InputLayoutT*   _sgInputButtons;


static inline void hkDisablePortInput(PORT_t* port) {
    HTRACE("input.c -> hkDisablePortInput(PORT_t*):void");

    port->DIRCLR = 0xFF;
    volatile u8* pincfg = &port->PIN0CTRL;
    for(u8 i = 0; i < 8; i++) pincfg[i] = PORT_ISC_INPUT_DISABLE_gc;
}


b8 hkInitInput(PlatformStateT* platformState, InputLayoutT* input) {
    HTRACE("input.c -> hkInitInput(PlatformStateT*, InputLayoutT*):b8");

    _sgInputPlatformState = platformState;
    _sgInputButtons       = input;

    PL_SET_RDY(platformState->statusFlags, PL_INPUT);
    hkInputConfig(input);

    return TRUE;
}

void hkStopInput() {
    HTRACE("input.c -> hkStopInput(void):void");
    HDEBUG("hkStopInput(): Stopping input subsystem.");

    hkDisablePortInput(&PORTA);
    // hkDisablePortInput(&PORTB);
    hkDisablePortInput(&PORTC);
    hkDisablePortInput(&PORTD);
    hkDisablePortInput(&PORTE);
    hkDisablePortInput(&PORTF);

    PL_SET_FLAGGED(_sgInputPlatformState->statusFlags, PL_INPUT);
    cli();

    HINFO("Inuput subsytem has been stopped.");
}


void hkInputConfig(InputLayoutT* input) {
    HTRACE("input.c -> hkInputConfig(PORT_t*, u16):void");

    if(!PL_IS_RDY(_sgInputPlatformState->statusFlags, PL_INPUT)) {
        HERROR("hkInputConfig(): Input subsystem mus be initialized first!");
        PL_SET_FLAGGED(_sgInputPlatformState->statusFlags, PL_GENERAL_ERROR);
        return;
    }

    cli();
    input->port->DIRCLR = input->pinMask;
    HDEBUG("Pin 0x%x as input", input->pinMask);

    volatile u8* pinCfg = &input->port->PIN0CTRL;               // Address of PIN0CTRL register
    for(u8 i = 0; i != 8; ++i) {                                // Iterate over registers
        if(input->pinMask & (1 << i)) pinCfg[i] = input->isc;   // Check which pin was passed to the function and set the ISC
    }

    input->port->INTFLAGS = input->pinMask;
    sei();
    
    return;
}



ISR(PORTA_PORT_vect) {
    u8 flags       = PORTA.INTFLAGS;
    PORTA.INTFLAGS = flags;

    for(u8 i = 0; i != BTN_COUNT; ++i) {
        const InputLayoutT *btn = &_sgInputButtons[i];
        if(flags & btn->pinMask) {            
            b8 pressed = !(btn->port->IN & btn->pinMask);

            EventT event;
            event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
            event.sender  = NULL;    // Or &whateverHere
            event.data[0] = btn->id;
            event.data[1] = pressed;

            hkEventFire(&event);
        }
    }
}

ISR(PORTB_PORT_vect) {
    u8 flags       = PORTB.INTFLAGS;
    PORTB.INTFLAGS = flags;

    for(u8 i = 0; i != BTN_COUNT; ++i) {
        const InputLayoutT *btn = &_sgInputButtons[i];
        if(flags & btn->pinMask) {            
            b8 pressed = !(btn->port->IN & btn->pinMask);

            EventT event;
            event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
            event.sender  = NULL;    // Or &whateverHere
            event.data[0] = btn->id;
            event.data[1] = pressed;

            hkEventFire(&event);
        }
    }
}

ISR(PORTC_PORT_vect) {
    u8 flags       = PORTC.INTFLAGS;
    PORTC.INTFLAGS = flags;

    for(u8 i = 0; i != BTN_COUNT; ++i) {
        const InputLayoutT *btn = &_sgInputButtons[i];
        if(flags & btn->pinMask) {            
            b8 pressed = !(btn->port->IN & btn->pinMask);

            EventT event;
            event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
            event.sender  = NULL;    // Or &whateverHere
            event.data[0] = btn->id;
            event.data[1] = pressed;

            hkEventFire(&event);
        }
    }
}

ISR(PORTD_PORT_vect) {
    u8 flags       = PORTD.INTFLAGS;
    PORTD.INTFLAGS = flags;

    for(u8 i = 0; i != BTN_COUNT; ++i) {
        const InputLayoutT *btn = &_sgInputButtons[i];
        if(flags & btn->pinMask) {            
            b8 pressed = !(btn->port->IN & btn->pinMask);

            EventT event;
            event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
            event.sender  = NULL;    // Or &whateverHere
            event.data[0] = btn->id;
            event.data[1] = pressed;

            hkEventFire(&event);
        }
    }
}

ISR(PORTE_PORT_vect) {
    u8 flags       = PORTE.INTFLAGS;
    PORTE.INTFLAGS = flags;

    for(u8 i = 0; i != BTN_COUNT; ++i) {
        const InputLayoutT *btn = &_sgInputButtons[i];
        if(flags & btn->pinMask) {            
            b8 pressed = !(btn->port->IN & btn->pinMask);

            EventT event;
            event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
            event.sender  = NULL;    // Or &whateverHere
            event.data[0] = btn->id;
            event.data[1] = pressed;

            hkEventFire(&event);
        }
    }
}

ISR(PORTF_PORT_vect) {
    u8 flags       = PORTF.INTFLAGS;
    PORTF.INTFLAGS = flags;

    for(u8 i = 0; i != BTN_COUNT; ++i) {
        const InputLayoutT *btn = &_sgInputButtons[i];
        if(flags & btn->pinMask) {            
            b8 pressed = !(btn->port->IN & btn->pinMask);

            EventT event;
            event.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
            event.sender  = NULL;    // Or &whateverHere
            event.data[0] = btn->id;
            event.data[1] = pressed;

            hkEventFire(&event);
        }
    }
}