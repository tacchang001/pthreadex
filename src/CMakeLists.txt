
set(SOURCES
        version.c
        error.c
        stderror.c
        tsk.c
        itc.c
        mempool.c
        queue.c
        )

set(DEFS
        ${PROJECT_SOURCE_DIR}/include/pthreadex.h
        ${PROJECT_SOURCE_DIR}/include/ele_task.h
        ${PROJECT_SOURCE_DIR}/include/queue.h

        )

set(HEADERS
        ${DEFS}
        version.h
        tsk.h
        mempool.h
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
