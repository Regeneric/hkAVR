cmake_minimum_required(VERSION 3.17)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build (Debug or Release)" FORCE)
endif()


set(PICO_BOARD pico2)
set(PICO_PLATFORM rp2350-riscv)     # RISC-V
# set(PICO_PLATFORM rp2350-arm-s)   # ARM

set(PICO_SDK_PATH ${CMAKE_SOURCE_DIR}/pico-sdk)
set(PICO_EXTRAS_PATH ${CMAKE_SOURCE_DIR}/pico-extras)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

project(hkRISCv C CXX ASM)

pico_sdk_init()

message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
# message(STATUS "Compiler flags: ${CMAKE_C_FLAGS}")

# Use C11 everywhere
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Use C++17 everywhere
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile_commands.json for VSCode/IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Public headers
set(PROJECT_INC_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${PROJECT_INC_DIR})


option(BUILD_FOR_AVR "Build for AVR platform" OFF)          # AVR build target
option(BUILD_FOR_ARM "Build for ARM platform" OFF)          # RP2040/RP2350 ARM build target
option(BUILD_FOR_RISCV "Build for RISC-V platform" OFF)     # RP2350 RISC-V build target

if(BUILD_FOR_AVR)
    add_compile_definitions(HPLATFORM_AVR)
    add_compile_definitions(__avr_gcc__)

    add_compile_definitions(HPLATFORM_AVR_4809)     # TODO: FOR NOW IT'S HARDCODED
elseif(BUILD_FOR_ARM)
    add_compile_definitions(HPLATFORM_ARM)
    add_compile_definitions(__arm_gcc__)
elseif(BUILD_FOR_RISCV)
    add_compile_definitions(HPLATFORM_RISCV)
    add_compile_definitions(__riscv_gcc__)
else()
    add_compile_definitions(HPLATFORM_UNKNOWN)
endif()


option(_DEBUG "Enable debug functions in code" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
    add_compile_definitions(DEBUG)
endif()


add_compile_definitions(HEXPORT)  # TODO: for now it's hardcoded

add_compile_options(
    -Wno-format                   # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
)


# External libs
add_subdirectory(lib/hkmalloc)

# Source files
add_subdirectory(src)


if(CMAKE_EXPORT_COMPILE_COMMANDS)
  file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/.vscode")

  add_custom_target(
    copy_compile_commands ALL
    COMMENT "⋯ copying compile_commands.json into .vscode/ ⋯"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/.vscode/compile_commands.json"
  )
endif()
