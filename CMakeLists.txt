cmake_minimum_required(VERSION 3.7)

set(PROJECT_NAME pthreadex)
project(${PROJECT_NAME})

# cmake -DCMAKE_VERBOSE_MAKEFILE=ON .
set(CMAKE_VERBOSE_MAKEFILE ON)

# Names
set(APP_NAME examples)
set(LIB_NAME pthreadex)
set(TEST_APP_NAME ${LIB_NAME}_tests)

# Switch build target
set(BUILD_TARGET PC CACHE STRING "Build target?")

# Build settings for each target
if(${BUILD_TARGET} STREQUAL PC)
    message("[BUILD] PC Linux")
    include(cmakes/PC_LINUX.cmake)
elseif(${BUILD_TARGET} STREQUAL RASPI_NATIVE)
    message("[BUILD] Raspberry Pi Native")
    include(cmakes/RASPI_NATIVE.cmake)
elseif(${BUILD_TARGET} STREQUAL RASPI_CROSS)
    message("[BUILD] Raspberry Pi Cross")
    include(cmakes/RASPI_CROSS.cmake)
else()
    message(FATAL_ERROR "[BUILD] Invalid target")
endif()

# Common build settings
include(cmakes/common.cmake)

# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/output)

# Profiling
option(COMPILE_GCOV "Coverage measurement" ON)
if(COMPILE_GCOV)
    SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

    SET(GCC_COVERAGE_LINK_FLAGS "-lgcov")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
endif(COMPILE_GCOV)

# 各プロジェクトをadd_subdirectory
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/${APP_NAME})

option(COMPILE_TESTS "Compile the tests" ON)
if(COMPILE_TESTS)
    message("[TEST] ON")
    find_package(PkgConfig)
    pkg_check_modules(PCUNIT REQUIRED pcunit)
    message(STATUS "PCUNIT_FOUND : ${PCUNIT_FOUND}")
    message(STATUS "PCUNIT_INCLUDE_DIRS : ${PCUNIT_INCLUDE_DIRS}")
    message(STATUS "PCUNIT_LIBRARIES : ${PCUNIT_LIBRARIES}")

    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif(COMPILE_TESTS)

