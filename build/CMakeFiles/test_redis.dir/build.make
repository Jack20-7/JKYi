# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/admin/workSpace

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/admin/workSpace/build

# Include any dependencies generated for this target.
include CMakeFiles/test_redis.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test_redis.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_redis.dir/flags.make

CMakeFiles/test_redis.dir/tests/test_redis.cc.o: CMakeFiles/test_redis.dir/flags.make
CMakeFiles/test_redis.dir/tests/test_redis.cc.o: ../tests/test_redis.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/admin/workSpace/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_redis.dir/tests/test_redis.cc.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_redis.dir/tests/test_redis.cc.o -c /home/admin/workSpace/tests/test_redis.cc

CMakeFiles/test_redis.dir/tests/test_redis.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_redis.dir/tests/test_redis.cc.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/admin/workSpace/tests/test_redis.cc > CMakeFiles/test_redis.dir/tests/test_redis.cc.i

CMakeFiles/test_redis.dir/tests/test_redis.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_redis.dir/tests/test_redis.cc.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/admin/workSpace/tests/test_redis.cc -o CMakeFiles/test_redis.dir/tests/test_redis.cc.s

# Object files for target test_redis
test_redis_OBJECTS = \
"CMakeFiles/test_redis.dir/tests/test_redis.cc.o"

# External object files for target test_redis
test_redis_EXTERNAL_OBJECTS =

../bin/test_redis: CMakeFiles/test_redis.dir/tests/test_redis.cc.o
../bin/test_redis: CMakeFiles/test_redis.dir/build.make
../bin/test_redis: ../lib/libJKYi.so
../bin/test_redis: CMakeFiles/test_redis.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/admin/workSpace/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/test_redis"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_redis.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_redis.dir/build: ../bin/test_redis

.PHONY : CMakeFiles/test_redis.dir/build

CMakeFiles/test_redis.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_redis.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_redis.dir/clean

CMakeFiles/test_redis.dir/depend:
	cd /home/admin/workSpace/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/admin/workSpace /home/admin/workSpace /home/admin/workSpace/build /home/admin/workSpace/build /home/admin/workSpace/build/CMakeFiles/test_redis.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_redis.dir/depend
