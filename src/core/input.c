#include <core/input.h>
#include <core/logger.h>
#include <platform/platform.h>

#include <avr/interrupt.h>
#include <avr/io.h>


ISR(PORTC_PORT_vect) {
    if (PORTC.INTFLAGS & BTN1_MASK) {
        // clear the flag by writing 1
        PORTC.INTFLAGS = BTN1_MASK;

        // read the pin (active low assumed)
        b8 pressed = !(PORTC.IN & BTN1_MASK);

        EventT e;
        e.code    = pressed ? EC_BTN_PRESSED : EC_BTN_RELEASED;
        e.sender  = NULL;            // or &yourButtonDriver
        e.data[0] = BTN_ID_1;        // which button
        e.data[1] = pressed;         // 1=down, 0=up

        hkEventFire(&e);
    }
    // if you have BTN2, do another `if(PORTB.INTFLAGS & BTN2_MASK)` here…
}

b8 hkInitInput(PlatformStateT* platformState) {
    HTRACE("input.c -> hkInitInput(void):b8");

    // 1) PB2 as input w/ pull-up
    PORTC.DIRCLR   = BTN1_MASK;
    PORTC.PIN4CTRL = PORT_PULLUPEN_bm      /* pull-up */
                   | PORT_ISC_BOTHEDGES_gc;/* both-edges detect */

    // 2) clear any pending PB2‐flags
    PORTC.INTFLAGS = BTN1_MASK;

    PL_SET_RDY(platformState->statusFlags, PL_INPUT);
    return TRUE;
}