add_library(hkcore STATIC
    event.c
    hkmemory.c
    hkstring.c
    input.c
    logger.c
)

if(BUILD_FOR_AVR)
    add_compile_definitions(HPLATFORM_AVR)
    add_compile_definitions(__avr_gcc__)
elseif(BUILD_FOR_ARM)
    add_compile_definitions(HPLATFORM_ARM)
    add_compile_definitions(__arm_gcc__)
elseif(BUILD_FOR_RISCV)
    add_compile_definitions(HPLATFORM_RISCV)
    add_compile_definitions(__riscv_gcc__)
else()
    message(WARNING "Unknown platform — no compile options set")
endif()

option(_DEBUG "Enable debug functions in code" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
    add_compile_definitions(DEBUG)
endif()


add_compile_definitions(HEXPORT)  # TODO: for now it's hardcoded

target_include_directories(hkcore
    PUBLIC
        ${PROJECT_INC_DIR}
        ${CMAKE_SOURCE_DIR}/lib/hkmalloc
)

# target_link_libraries(hkcore
#     PUBLIC
#         pico_stdio_usb 
# )