file(GLOB_RECURSE APP_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

list(REMOVE_ITEM APP_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/platform/platform_avr_4809.c")

add_executable(${PROJECT_NAME} ${APP_SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}    
    pico_stdlib 
    pico_multicore
    pico_binary_info

    hardware_gpio
    hardware_irq
    hardware_sync
    hardware_flash
    # hardware_i2c

    # pico_cyw43_arch_none 
    # pico_lwipallapps 
    # pico_lwipcore

    hkmalloc
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})