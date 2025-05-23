add_library(hkcore STATIC
    event.c
    hkmemory.c
    hkstring.c
    input.c
    logger.c
)

target_include_directories(hkcore
    PUBLIC
        ${PROJECT_INC_DIR}
        ${CMAKE_SOURCE_DIR}/lib/hkmalloc
)