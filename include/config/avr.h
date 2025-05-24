#pragma once
#include <defines.h>

#if HPLATFORM_AVR
    // PLATFOM SETTINGS
    #define HPLATFORM_AVR_4809                 // ATMega 4809

    // GENERAL CONFIG
    #define HK_USE_CRLF            TRUE        // Use \r\n as a newline character
    #define HK_TIMER_SLEEP         TRUE        // Use TIMER0 for sleep instead of _delay_ms()
    #define HK_USE_SCREEN          FALSE       // If you wanna use some LCD or OLED
    #define HK_BAUD_RATE           115200      // Terminal baud rate
    #define HK_USE_PROGMEM         TRUE        // Use PROGMEM when it is possible
    #define HK_LOG_MSG_MAX_LEN     80          // Maximum length of a single log message
    #define HK_KEEP_TRACK_OF_TIME  FALSE       // Keep accurate track of time since uC has started
    #define HK_LOG_LEVEL           LOG_DEBUG   // TRACE | DEBUG | INFO | WARN | ERROR
    #define HK_USE_ISR_INPUT       FALSE       // Use hardwawre interrupts to read buttons - NO EFFECT FOR NOW
    #define HK_LOG_OUTPUT          UART        // UART | USB - NO EFFECT FOR NOW

    // INPUT
    #define HK_INPUT_REG           PORTC       // Input register
    #define HK_ACCEPT_BTN          PIN4_bm     // Accept button
    #define HK_CANCEL_BTN          PIN5_bm     // Cancel button
    #define HK_NEXT_BTN            PIN6_bm     // Next button
    #define HK_PREV_BTN            PIN7_bm     // Previous button

    // To add more buttons just expand this list
    // ACCEPT is ID and HK_ACCEP_BTN is a physicall PIN of the uC
    #define HK_BUTTON_LIST          \
        X(ACCEPT,   HK_ACCEPT_BTN)  \
        X(CANCEL,   HK_CANCEL_BTN)  \
        X(NEXT  ,   HK_NEXT_BTN)    \
        X(PREV  ,   HK_PREV_BTN)
    #define HK_MAX_BUTTONS       ((sizeof((int[]){HK_BUTTON_LIST})/sizeof(int)))

    // USE AT OWN RISK 
    #define HK_ALLOW_MALLOC        TRUE        // Allow malloc()
    #define HK_MEM_HEAVY_DBG       FALSE       // There are some memory heavy debug functions
#endif