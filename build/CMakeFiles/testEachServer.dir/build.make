# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/luo/cplus/gaiya

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/luo/cplus/gaiya/build

# Include any dependencies generated for this target.
include CMakeFiles/testEachServer.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/testEachServer.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/testEachServer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testEachServer.dir/flags.make

CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.o: CMakeFiles/testEachServer.dir/flags.make
CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.o: /home/luo/cplus/gaiya/tester/testEachServer.cpp
CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.o: CMakeFiles/testEachServer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/luo/cplus/gaiya/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.o -MF CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.o.d -o CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.o -c /home/luo/cplus/gaiya/tester/testEachServer.cpp

CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/luo/cplus/gaiya/tester/testEachServer.cpp > CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.i

CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/luo/cplus/gaiya/tester/testEachServer.cpp -o CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.s

# Object files for target testEachServer
testEachServer_OBJECTS = \
"CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.o"

# External object files for target testEachServer
testEachServer_EXTERNAL_OBJECTS =

/home/luo/cplus/gaiya/bin/testEachServer: CMakeFiles/testEachServer.dir/tester/testEachServer.cpp.o
/home/luo/cplus/gaiya/bin/testEachServer: CMakeFiles/testEachServer.dir/build.make
/home/luo/cplus/gaiya/bin/testEachServer: /home/luo/cplus/gaiya/lib/libgaiya.so
/home/luo/cplus/gaiya/bin/testEachServer: CMakeFiles/testEachServer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/luo/cplus/gaiya/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/luo/cplus/gaiya/bin/testEachServer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testEachServer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testEachServer.dir/build: /home/luo/cplus/gaiya/bin/testEachServer
.PHONY : CMakeFiles/testEachServer.dir/build

CMakeFiles/testEachServer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testEachServer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testEachServer.dir/clean

CMakeFiles/testEachServer.dir/depend:
	cd /home/luo/cplus/gaiya/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/luo/cplus/gaiya /home/luo/cplus/gaiya /home/luo/cplus/gaiya/build /home/luo/cplus/gaiya/build /home/luo/cplus/gaiya/build/CMakeFiles/testEachServer.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/testEachServer.dir/depend

