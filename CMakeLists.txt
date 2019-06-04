cmake_minimum_required(VERSION 3.1)
project(libcanard)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

## only compatible with 32 bits and less architecture
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
#     add_definitions(-m32)
    set(CMAKE_C_FLAGS -m32)
endif()

set(CANARD_LIBNAME canard)
set(DRV_SOCK_LIBNAME canard_socketcan)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-ggdb -O0)
else()
    add_definitions(-O2)
endif()

if(NOT "${OPT_ROOT_DIR}" STREQUAL "")
    message(STATUS "Use OPT_ROOT_DIR: ${OPT_ROOT_DIR}")
    include_directories(${OPT_ROOT_DIR}/include)
    link_directories(${OPT_ROOT_DIR}/lib/)
endif()

add_subdirectory(src)
add_subdirectory(test/sample)
