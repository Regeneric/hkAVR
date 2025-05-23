set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_C_COMPILER   avr-gcc)
set(CMAKE_C_FLAGS          "-mmcu=atmega4809 -std=gnu11 -DF_CPU=20000000UL")
set(CMAKE_EXE_LINKER_FLAGS "-mmcu=atmega4809 -Wl,-u,vfprintf -Wl,-u,vsnprintf -lprintf_flt -lm")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)