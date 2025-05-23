add_library(hkcontainers STATIC
    darray.c
)

target_include_directories(hkcontainers
    PUBLIC
        ${PROJECT_INC_DIR}
)