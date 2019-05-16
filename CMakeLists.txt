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

add_subdirectory(src)
add_subdirectory(test/sample)
