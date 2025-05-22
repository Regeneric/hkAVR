add_library(hkmalloc STATIC hkmalloc.c)
target_include_directories(hkmalloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})