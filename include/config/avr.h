#pragma once
#if HPLATFORM_AVR
    #define USE_CRLF            FALSE       // Use \r\n as a newline character
    #define TIMER_SLEEP         TRUE        // Use TIMER0 for sleep instead of _delay_ms()
    #define USE_SCREEN          FALSE       // If you wanna use some LCD or OLED
    #define BAUD_RATE           115200      // Terminal baud rate
    #define USE_PROGMEM         TRUE        // Use PROGMEM when it is possible
    #define LOG_MSG_MAX_LEN     80          // Maximum length of a single log message ()
    #define KEEP_TRACK_OF_TIME  FALSE       // Keep accurate track of time sinc uC has started
    #define LOG_LEVEL           LOG_TRACE   // TRACE | DEBUG | INFO | WARN | ERROR
    #define USE_ISR_INPUT       FALSE       // Use hardwawre interrupts to read buttons

    // USE AT OWN RISK 
    #define ALLOW_MALLOC        FALSE       // Allow malloc()
    #define MEM_HEAVY_DBG       FALSE       // There are some memory heavy debug functions
#endif