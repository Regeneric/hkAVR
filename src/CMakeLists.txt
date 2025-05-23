add_subdirectory(containers)
add_subdirectory(core)
add_subdirectory(platform)

add_executable(${PROJECT_NAME} main.c)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INC_DIR})
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        hkcontainers
        hkcore
        hkplatform
        hkmalloc
)


if(BUILD_FOR_AVR)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-O0 -g -Wall -ffunction-sections -fdata-sections -ffreestanding>
        $<$<CONFIG:Release>:-Os -ffunction-sections -fdata-sections -ffreestanding>
    )
elseif(BUILD_FOR_ARM)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-O0 -g -std=gnu11>
        $<$<CONFIG:Release>:-Os -std=gnu11>
    )
elseif(BUILD_FOR_RISCV)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-O0 -g -std=gnu11>
        $<$<CONFIG:Release>:-Os -std=gnu11>
    )
else()
    message(WARNING "Unknown platform — no compile options set")
endif()