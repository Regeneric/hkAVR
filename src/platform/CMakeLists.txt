set(PLATFORM_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_FOR_AVR)
    add_compile_definitions(HPLATFORM_AVR)
    add_compile_definitions(__avr_gcc__)

    file(GLOB PLATFORM_SOURCES
        ${PLATFORM_DIR}/platform_avr*.c
    )
elseif(BUILD_FOR_ARM)
    add_compile_definitions(HPLATFORM_ARM)
    add_compile_definitions(__arm_gcc__)

    file(GLOB PLATFORM_SOURCES
        ${PLATFORM_DIR}/platform_arm*.c
    )
elseif(BUILD_FOR_RISCV)
    add_compile_definitions(HPLATFORM_RISCV)
    add_compile_definitions(__riscv_gcc__)
    
    file(GLOB PLATFORM_SOURCES
        ${PLATFORM_DIR}/platform_riscv*.c
    )
else()
    message(FATAL_ERROR "No platform selected - please turn on BUILD_FOR_AVR, BUILD_FOR_ARM or BUILD_FOR_RISCV")
endif()

option(_DEBUG "Enable debug functions in code" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
    add_compile_definitions(DEBUG)
endif()


add_compile_definitions(HEXPORT)  # TODO: for now it's hardcoded

add_library(hkplatform STATIC
    ${PLATFORM_SOURCES}
)

target_include_directories(hkplatform
    PUBLIC
        ${PROJECT_INC_DIR}
        ${CMAKE_SOURCE_DIR}/lib/hkmalloc
)

target_link_libraries(hkplatform
    PUBLIC
        pico_stdlib
        pico_stdio_uart
        pico_stdio_usb 
        pico_multicore
        
        hardware_uart
)