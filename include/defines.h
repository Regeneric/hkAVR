#pragma once

#include <stdint.h>
#include <stddef.h>

#define TRUE  1
#define FALSE 0


// Standard unsigned types
typedef uint8_t        u8;
typedef uint16_t       u16;
typedef uint32_t       u32;
typedef uint64_t       u64;

// Best runtime performance - heavier on RAM 
typedef uint_fast8_t   fu8;
typedef uint_fast16_t  fu16;
typedef uint_fast32_t  fu32;
typedef uint_fast64_t  fu64;

// Easier on RAM - worse runtime performance
typedef uint_least8_t  lu8;
typedef uint_least16_t lu16;
typedef uint_least32_t lu32;
typedef uint_least64_t lu64;   


// Standard signed types
typedef int8_t         i8;
typedef int16_t        i16;
typedef int32_t        i32;
typedef int64_t        i64;

// Best runtime performance - heavier on RAM 
typedef int_fast8_t    fi8;
typedef int_fast16_t   fi16;
typedef int_fast32_t   fi32;
typedef int_fast64_t   fi64;

// Easier on RAM - worse runtime performance
typedef int_least8_t   li8;
typedef int_least16_t  li16;
typedef int_least32_t  li32;
typedef int_least64_t  li64;


typedef float          f32;
typedef double         f64;

typedef int8_t         b8;
typedef int32_t        b32;


#if defined(__GNUC__) && defined(__AVR__)
    #ifndef __avr_gcc__
        #define __avr_gcc__
    #endif
#endif

#if defined(__GNUC__) && (defined(__arm__) || defined(__aarch64__) || defined(__thumb__))
    #ifndef __arm_gcc__
        #define __arm_gcc__
    #endif
#endif

#if defined(__GNUC__) && defined(__riscv)
    #ifndef __riscv_gcc__   
        #define __riscv_gcc__  
    #endif

    #if defined(__riscv_xlen) && (__riscv_xlen == 64)
        #ifndef __riscv64_gcc__
            #define __riscv64_gcc__
        #endif
    #else
        #ifndef __riscv32_gcc__
            #define __riscv32_gcc__
        #endif
    #endif
#endif


#if defined(__clang__) || defined(__gcc__) || defined(__avr_gcc__) || defined(__arm_gcc__) || defined(__riscv_gcc__)
    #define STATIC_ASSERT _Static_assert
#else
    #define STATIC_ASSERT static_assert
#endif

STATIC_ASSERT(sizeof(u8)  == 1, "Expected u8 to be 1 byte");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes");

STATIC_ASSERT(sizeof(i8)  == 1, "Expected i8 to be 1 byte");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes");

#if __SIZEOF_DOUBLE__ != 8
    #ifdef _MSC_VER
        #pragma message "WARNING: `double` is less than 8 bytes!"
    #else
        #warning WARNING: `double` is less than 8 bytes!
    #endif
#else
    STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes");
#endif

#if defined(HPLATFORM_ARM)
    #ifndef HPLATFORM_ARM
        #define HPLATFORM_ARM  TRUE
    #endif
    #include "config/arm.h"
#elif defined(HPLATFORM_RISCV)
    #ifndef HPLATFORM_RISCV
        #define HPLATFORM_RISCV TRUE
    #endif
    #include "config/riscv.h"
#elif defined(HPLATFORM_AVR)       
    #ifndef HPLATFORM_AVR
        #define HPLATFORM_AVR TRUE
    #endif
    #include "config/avr.h"                                       
#else
    #error "Unknown platform — please add your own HPLATFORM_*"
#endif

// Export
#ifdef HEXPORT
    #ifdef _MSC_VER
        #define HAPI __declspec(dllexport)
    #else
        #define HAPI __attribute__((visibility("default")))
    #endif
#else   
// Import
    #ifdef _MSC_VER
        #define HAPI __declspec(dllimport)
    #else
        #define HAPI
    #endif
#endif



#if defined(_DEBUG) && defined(HPLATFORM_AVR) 
    #include <avr/io.h>
    #include <util/delay.h>

    #ifdef HPLATFORM_AVR_4809
        #define PLATFORM_DBG    PIN0_bm
        #define APP_DBG         PIN1_bm
        #define MEM_DBG         PIN0_bm
        #define TX_DBG          PIN3_bm

        #define FLAG_PLATFORM  (PORTB.OUTSET = PLATFORM_DBG)
        #define _FLAG_PLATFORM (PORTB.OUTCLR = PLATFORM_DBG)

        #define FLAG_APP       (PORTB.OUTSET = APP_DBG)
        #define _FLAG_APP      (PORTB.OUTCLR = APP_DBG)
        
        #define FLAG_MEM       (PORTB.OUTSET = MEM_DBG)
        #define _FLAG_MEM      (PORTB.OUTCLR = MEM_DBG)
        
        #define FLAG_TX        (PORTE.OUTSET = TX_DBG)
        #define _FLAG_TX       (PORTE.OUTCLR = TX_DBG)

        static inline void hkDebugRegister(void) {
            PORTB.DIRSET = PLATFORM_DBG | APP_DBG;
            PORTB.OUTCLR = PLATFORM_DBG | APP_DBG;

            PORTE.DIRSET = TX_DBG | MEM_DBG;
            PORTE.OUTCLR = TX_DBG | MEM_DBG;
        }

        __attribute__((noreturn))
        static inline void hkHardwareDebug(u32 flags) {
            _delay_ms(1000);

            for (u8 i = 0; i < 32; ++i) {
                if(flags & (UINT32_C(1) << i)) PORTB.OUTSET = PLATFORM_DBG;
                else PORTB.OUTCLR = PLATFORM_DBG;

                // Short flash for each bit
                _delay_ms(1000);
            } PORTB.OUTCLR = PLATFORM_DBG;

            while(1);
        }

        #define REGISTER_DEBUG()      hkDebugRegister()
        #define HARDWARE_DEBUG(flags) hkHardwareDebug(flags)
    #endif
#else
    #define PLATFORM_DBG
    #define APP_DBG
    #define MEM_DBG
    #define TX_DBG 

    #define FLAG_PLATFORM
    #define FLAG_APP 
    #define FLAG_MEM
    #define FLAG_TX 

    #define _FLAG_PLATFORM
    #define _FLAG_APP
    #define _FLAG_MEM
    #define _FLAG_TX
    
    #define REGISTER_DEBUG()
    #define HARDWARE_DEBUG(flags)
#endif