
# Names
set(EXAMPLE_QUEUE example_timer)


set(SOURCES
        main.c
        )

set(HEADERS
        # main.h
        )

add_executable(${EXAMPLE_QUEUE}
        ${SOURCES}
        ${HEADERS}
        )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -std=gnu99")
#target_compile_options(${EXAMPLE_QUEUE}
#     PRIVATE ${CMAKE_C_FLAGS}
#     )

target_include_directories(${EXAMPLE_QUEUE}
        PRIVATE ../../include
        )

find_library(PCUNIT_LIBRARY ${PCUNIT_LIBRARIES} /home/devel/.local/lib)
target_link_libraries(${EXAMPLE_QUEUE} ${PCUNIT_LIBRARY} ${LIB_NAME} m)

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(${EXAMPLE_QUEUE} PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(${EXAMPLE_QUEUE} "${CMAKE_THREAD_LIBS_INIT}")
endif()
message(STATUS "CMAKE_THREAD_LIBS_INIT : ${CMAKE_THREAD_LIBS_INIT}")
