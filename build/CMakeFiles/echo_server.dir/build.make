# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_SOURCE_DIR = /home/zsj/workspace/sylar

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zsj/workspace/sylar/build

# Include any dependencies generated for this target.
include CMakeFiles/echo_server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/echo_server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/echo_server.dir/flags.make

CMakeFiles/echo_server.dir/examples/echo_server.cpp.o: CMakeFiles/echo_server.dir/flags.make
CMakeFiles/echo_server.dir/examples/echo_server.cpp.o: ../examples/echo_server.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zsj/workspace/sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/echo_server.dir/examples/echo_server.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) -D__FILE__=\"examples/echo_server.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/echo_server.dir/examples/echo_server.cpp.o -c /home/zsj/workspace/sylar/examples/echo_server.cpp

CMakeFiles/echo_server.dir/examples/echo_server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/echo_server.dir/examples/echo_server.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"examples/echo_server.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zsj/workspace/sylar/examples/echo_server.cpp > CMakeFiles/echo_server.dir/examples/echo_server.cpp.i

CMakeFiles/echo_server.dir/examples/echo_server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/echo_server.dir/examples/echo_server.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"examples/echo_server.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zsj/workspace/sylar/examples/echo_server.cpp -o CMakeFiles/echo_server.dir/examples/echo_server.cpp.s

CMakeFiles/echo_server.dir/examples/echo_server.cpp.o.requires:

.PHONY : CMakeFiles/echo_server.dir/examples/echo_server.cpp.o.requires

CMakeFiles/echo_server.dir/examples/echo_server.cpp.o.provides: CMakeFiles/echo_server.dir/examples/echo_server.cpp.o.requires
	$(MAKE) -f CMakeFiles/echo_server.dir/build.make CMakeFiles/echo_server.dir/examples/echo_server.cpp.o.provides.build
.PHONY : CMakeFiles/echo_server.dir/examples/echo_server.cpp.o.provides

CMakeFiles/echo_server.dir/examples/echo_server.cpp.o.provides.build: CMakeFiles/echo_server.dir/examples/echo_server.cpp.o


# Object files for target echo_server
echo_server_OBJECTS = \
"CMakeFiles/echo_server.dir/examples/echo_server.cpp.o"

# External object files for target echo_server
echo_server_EXTERNAL_OBJECTS =

../bin/echo_server: CMakeFiles/echo_server.dir/examples/echo_server.cpp.o
../bin/echo_server: CMakeFiles/echo_server.dir/build.make
../bin/echo_server: ../lib/libsylar.so
../bin/echo_server: /usr/lib/x86_64-linux-gnu/libz.so
../bin/echo_server: /usr/lib/x86_64-linux-gnu/libssl.so
../bin/echo_server: /usr/lib/x86_64-linux-gnu/libcrypto.so
../bin/echo_server: CMakeFiles/echo_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zsj/workspace/sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/echo_server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/echo_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/echo_server.dir/build: ../bin/echo_server

.PHONY : CMakeFiles/echo_server.dir/build

CMakeFiles/echo_server.dir/requires: CMakeFiles/echo_server.dir/examples/echo_server.cpp.o.requires

.PHONY : CMakeFiles/echo_server.dir/requires

CMakeFiles/echo_server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/echo_server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/echo_server.dir/clean

CMakeFiles/echo_server.dir/depend:
	cd /home/zsj/workspace/sylar/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zsj/workspace/sylar /home/zsj/workspace/sylar /home/zsj/workspace/sylar/build /home/zsj/workspace/sylar/build /home/zsj/workspace/sylar/build/CMakeFiles/echo_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/echo_server.dir/depend

