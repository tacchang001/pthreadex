
set(SOURCES
        main.c
        test_tsk.c

        dft.c
        )

set(HEADERS
        misc.h
        )


add_executable(${TEST_APP_NAME}
        ${SOURCES}
        )

# target_compile_options(${TEST_APP_NAME}
#         PRIVATE ${CMAKE_CXX_FLAGS}
#         )

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PCUNIT_INCLUDE_DIRS}
)

find_library(PCUNIT_LIBRARY ${PCUNIT_LIBRARIES} /home/devel/.local/lib)
target_link_libraries(${TEST_APP_NAME} ${PCUNIT_LIBRARY} ${LIB_NAME} m)

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(${TEST_APP_NAME} PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(${TEST_APP_NAME} "${CMAKE_THREAD_LIBS_INIT}")
endif()
message(STATUS "CMAKE_THREAD_LIBS_INIT : ${CMAKE_THREAD_LIBS_INIT}")
