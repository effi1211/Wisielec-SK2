cmake_minimum_required(VERSION 3.10)
project(Wisielec)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")

set(SOURCES serwer.cpp)

add_executable(ser ${SOURCES})

add_custom_target(cppcheck
 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running cppcheck")


# add cppcheck as a dependency of main target
add_dependencies(ser cppcheck)