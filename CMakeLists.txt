cmake_minimum_required(VERSION 3.10)

project(csg-2025-assembly VERSION 1.0 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

include_directories(includes)

file(GLOB SOURCES src/*.cpp)
find_package(Curses REQUIRED)

add_executable(asm ${SOURCES})

target_include_directories(asm PRIVATE ${CURSES_INCLUDE_DIR})

target_link_libraries(asm PRIVATE ${CURSES_LIBRARIES})
