cmake_minimum_required(VERSION 3.7)
project(myThreadPool)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES
        #main.c
        thread_pool.c
        thread_pool.h
        queue.c
        queue.h
        #queue_test.c
        myhttpserver.c
        type.h
        )
add_executable(myThreadPool ${SOURCE_FILES})


TARGET_LINK_LIBRARIES( myThreadPool
        pthread
        )