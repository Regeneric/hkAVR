set(PLATFORM_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_FOR_AVR)
    file(GLOB PLATFORM_SOURCES
        ${PLATFORM_DIR}/platform_avr*.c
    )
elseif(BUILD_FOR_ARM)
    file(GLOB PLATFORM_SOURCES
        ${PLATFORM_DIR}/platform_arm*.c
    )
elseif(BUILD_FOR_RISCV)
    file(GLOB PLATFORM_SOURCES
        ${PLATFORM_DIR}/platform_riscv*.c
    )
else()
    message(FATAL_ERROR "No platform selected - please turn on BUILD_FOR_AVR, BUILD_FOR_ARM or BUILD_FOR_RISCV")
endif()

add_library(hkplatform STATIC
    ${PLATFORM_SOURCES}
)

target_include_directories(hkplatform
    PUBLIC
        ${PROJECT_INC_DIR}
        ${CMAKE_SOURCE_DIR}/lib/hkmalloc
)