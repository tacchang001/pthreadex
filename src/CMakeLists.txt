
set(SOURCES
        version.c
        error.c
        stderror.c
        tsk.c
        )

set(DEFS
        ${PROJECT_SOURCE_DIR}/include/pthreadex.h
        ${PROJECT_SOURCE_DIR}/include/ele_task.h
        )

set(HEADERS
        ${DEFS}
        version.h
        tsk.h
        mutex_lock.h
        )

add_library(${LIB_NAME} STATIC
        ${SOURCES}
        ${HEADERS}
        )

include_directories(${PROJECT_SOURCE_DIR}/include)

# target_compile_options(${LIB_NAME}
#         PRIVATE ${CMAKE_CXX_FLAGS}
#         )
