cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME hkAVRm)
set(PICO_BOARD pico2)
set(PICO_PLATFORM rp2350-riscv)     # RISC-V
# set(PICO_PLATFORM rp2350-arm-s)   # ARM

set(PICO_SDK_PATH    ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk)
set(PICO_EXTRAS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-extras)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

project(${PROJECT_NAME} C CXX ASM)
pico_sdk_init()

add_compile_options(
    -O0
    -g
    -std=gnu11

    -Wall
    -Wno-format                 # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function        # we have some for the docs that aren't called

    -DHPLATFORM_ARM_RP2350=1
    -DHEXPORT
    -D_DEBUG
)

add_subdirectory(lib/hkmalloc)
add_subdirectory(src)