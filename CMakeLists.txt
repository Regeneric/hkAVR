cmake_minimum_required(VERSION 3.17)
project(hkRISCv C)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build (Debug or Release)" FORCE)
endif()


# set(CMAKE_C_FLAGS         "-std=gnu11")
# set(CMAKE_C_FLAGS_DEBUG   "-O0 -g")
# set(CMAKE_C_FLAGS_RELEASE "-Os")


message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
# message(STATUS "Compiler flags: ${CMAKE_C_FLAGS}")

# Use C11 everywhere
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


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
