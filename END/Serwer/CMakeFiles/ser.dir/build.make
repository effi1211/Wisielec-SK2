# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/c/Users/Monks/Desktop/sk22/Serwer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Users/Monks/Desktop/sk22/Serwer

# Include any dependencies generated for this target.
include CMakeFiles/ser.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ser.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ser.dir/flags.make

CMakeFiles/ser.dir/serwer.cpp.o: CMakeFiles/ser.dir/flags.make
CMakeFiles/ser.dir/serwer.cpp.o: serwer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/Monks/Desktop/sk22/Serwer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ser.dir/serwer.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ser.dir/serwer.cpp.o -c /mnt/c/Users/Monks/Desktop/sk22/Serwer/serwer.cpp

CMakeFiles/ser.dir/serwer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ser.dir/serwer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/Monks/Desktop/sk22/Serwer/serwer.cpp > CMakeFiles/ser.dir/serwer.cpp.i

CMakeFiles/ser.dir/serwer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ser.dir/serwer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/Monks/Desktop/sk22/Serwer/serwer.cpp -o CMakeFiles/ser.dir/serwer.cpp.s

# Object files for target ser
ser_OBJECTS = \
"CMakeFiles/ser.dir/serwer.cpp.o"

# External object files for target ser
ser_EXTERNAL_OBJECTS =

ser: CMakeFiles/ser.dir/serwer.cpp.o
ser: CMakeFiles/ser.dir/build.make
ser: CMakeFiles/ser.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/c/Users/Monks/Desktop/sk22/Serwer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ser"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ser.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ser.dir/build: ser

.PHONY : CMakeFiles/ser.dir/build

CMakeFiles/ser.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ser.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ser.dir/clean

CMakeFiles/ser.dir/depend:
	cd /mnt/c/Users/Monks/Desktop/sk22/Serwer && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/Monks/Desktop/sk22/Serwer /mnt/c/Users/Monks/Desktop/sk22/Serwer /mnt/c/Users/Monks/Desktop/sk22/Serwer /mnt/c/Users/Monks/Desktop/sk22/Serwer /mnt/c/Users/Monks/Desktop/sk22/Serwer/CMakeFiles/ser.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ser.dir/depend
