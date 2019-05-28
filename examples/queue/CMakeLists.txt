
set(SOURCES
        main.c
        )

set(HEADERS
        # main.h
        )

add_executable(${APP_NAME}
        ${SOURCES}
        ${HEADERS}
        )

# target_compile_options(${APP_NAME}
#         PRIVATE ${CMAKE_CXX_FLAGS}
#         )

target_link_libraries(${APP_NAME}
        ${LIB_NAME}
        )

target_include_directories(${APP_NAME}
        PRIVATE ../../include
        )

