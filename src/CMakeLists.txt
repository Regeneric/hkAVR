add_subdirectory(containers)
add_subdirectory(core)
add_subdirectory(platform)

add_executable(${PROJECT_NAME} main.c)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INC_DIR})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        hkplatform
        hkcore
        hkcontainers
        hkmalloc

    PUBLIC
        pico_stdlib
        pico_stdio
        pico_multicore
        pico_stdio_usb
        # pico_binary_info

        hardware_gpio
        # hardware_irq
        # hardware_spi
        # hardware_i2c
        # hardware_flash
        # hardware_timer
        # hardware_sync

        # pico_cyw43_arch_none 
        # pico_lwipallapps 
        # pico_lwipcore
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})


if(BUILD_FOR_AVR)
    add_compile_definitions(HPLATFORM_AVR)
    add_compile_definitions(__avr_gcc__)

    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-O0 -g -Wall -ffunction-sections -fdata-sections -ffreestanding>
        $<$<CONFIG:Release>:-Os -ffunction-sections -fdata-sections -ffreestanding>
    )
elseif(BUILD_FOR_ARM)
    add_compile_definitions(HPLATFORM_ARM)
    add_compile_definitions(__arm_gcc__)

    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-O0 -g -std=gnu11>
        $<$<CONFIG:Release>:-Os -std=gnu11>
    )
elseif(BUILD_FOR_RISCV)
    add_compile_definitions(HPLATFORM_RISCV)
    add_compile_definitions(__riscv_gcc__)
    
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-O0 -g -std=gnu11>
        $<$<CONFIG:Release>:-Os -std=gnu11>
    )
else()
    message(WARNING "Unknown platform — no compile options set")
endif()

option(_DEBUG "Enable debug functions in code" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
    add_compile_definitions(DEBUG)
endif()


add_compile_definitions(HEXPORT)  # TODO: for now it's hardcoded