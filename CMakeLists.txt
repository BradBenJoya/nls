cmake_minimum_required(VERSION 3.28)
project(nls)

set(CMAKE_CXX_STANDARD 23)

add_executable(nls nls.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(nls PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(nls PRIVATE /W4)
endif()