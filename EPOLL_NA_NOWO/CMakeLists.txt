cmake_minimum_required(VERSION 3.10)  # CMake version check
project(wisielec)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)           # Enable c++14 standard

# Add main.cpp file of project root directory as source file
set(SOURCE_FILES serwer.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(main ${SOURCE_FILES})


# add cppcheck as a dependency of main target
add_dependencies(main cppcheck)