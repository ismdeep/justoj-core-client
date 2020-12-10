cmake_minimum_required(VERSION 2.8)
project(justoj-core C)

set(BRANCH_NAME "")
set(RELEASE_DATE "")
set(COMMIT_HASH "")

find_package(Git QUIET)

if(GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD
            OUTPUT_VARIABLE BRANCH_NAME
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%H
            OUTPUT_VARIABLE COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%at
            OUTPUT_VARIABLE RELEASE_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
message(STATUS "Git version is ${BRANCH_NAME}:${RELEASE_DATE}:${COMMIT_HASH}")

set(CMAKE_C_STANDARD 11)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBRANCH_NAME=\\\"${BRANCH_NAME}\\\"")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DRELEASE_DATE=${RELEASE_DATE}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCOMMIT_HASH=\\\"${COMMIT_HASH}\\\"")

include_directories("./include")

# git clone https://github.com/ismdeep/ismdeep-c-utils.git vendor/ismdeep-c-utils
include_directories("./vendor/ismdeep-c-utils/include")

# git clone https://github.com/ismdeep/log.h.git vendor/log.h
include_directories("./vendor/log.h/src")

add_executable(justoj-core        justoj_core.c)
add_executable(justoj-core-client justoj_core_client.c)

if (NOT JUSTOJ_INSTALL_DIST)
    set(JUSTOJ_INSTALL_DIST /usr/bin)
endif()

set(CMAKE_INSTALL_PREFIX ${JUSTOJ_INSTALL_DIST})

install(TARGETS justoj-core DESTINATION .)
install(TARGETS justoj-core-client DESTINATION .)
