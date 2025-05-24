#pragma once
#include <defines.h>

#ifdef HPLATFORM_RISCV
    // PLATFOM SETTINGS
    #define HPLATFORM_RISCV_2350               // RP2350 - Hazard3
    #define HLOG_UART              uart0       // uart0 | uart1 - NO EFFECT FOR NOW        
    #define HLOG_UART_TX           0           // GPIO0 - Pico default UART TX - NO EFFECT FOR NOW
    #define HLOG_UART_RX           1           // GPIO1 - Pico deafult UART RX - NO EFFECT FOR NOW       

    // GENERAL CONFIG
    #define HK_USE_CRLF            TRUE        // Use \r\n as a newline character
    #define HK_TIMER_SLEEP         FALSE       // Use TIMER0 for sleep instead of _delay_ms() - TODO: TO BE DBUGGED
    #define HK_USE_SCREEN          FALSE       // If you wanna use some LCD or OLED
    #define HK_BAUD_RATE           115200      // Terminal baud rate
    #define HK_LOG_MSG_MAX_LEN     80          // Maximum length of a single log message
    #define HK_LOG_LEVEL           LOG_TRACE   // TRACE | DEBUG | INFO | WARN | ERROR
    #define HK_USE_ISR_INPUT       FALSE       // Use hardwawre interrupts to read buttons - NO EFFECT FOR NOW
    #define HK_USB_LOG_OUTPUT      TRUE        // TRUE - USB ; FALSE - UART

    // INPUT
    #define HK_ACCEPT_BTN          2           // Accept button   - GPIO2
    #define HK_CANCEL_BTN          3           // Cancel button   - GPIO3
    #define HK_NEXT_BTN            4           // Next button     - GPIO4
    #define HK_PREV_BTN            5           // Previous button - GPIO5

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