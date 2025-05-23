add_library(hkmalloc STATIC
    hkmalloc.c
)

target_include_directories(hkmalloc
    PUBLIC
        ${CMAKE_SOURCE_DIR}/lib/hkmalloc
)