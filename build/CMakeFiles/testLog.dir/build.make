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
include CMakeFiles/testLog.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/testLog.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/testLog.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testLog.dir/flags.make

CMakeFiles/testLog.dir/tester/testLog.cpp.o: CMakeFiles/testLog.dir/flags.make
CMakeFiles/testLog.dir/tester/testLog.cpp.o: /home/luo/cplus/gaiya/tester/testLog.cpp
CMakeFiles/testLog.dir/tester/testLog.cpp.o: CMakeFiles/testLog.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/luo/cplus/gaiya/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/testLog.dir/tester/testLog.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testLog.dir/tester/testLog.cpp.o -MF CMakeFiles/testLog.dir/tester/testLog.cpp.o.d -o CMakeFiles/testLog.dir/tester/testLog.cpp.o -c /home/luo/cplus/gaiya/tester/testLog.cpp

CMakeFiles/testLog.dir/tester/testLog.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/testLog.dir/tester/testLog.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/luo/cplus/gaiya/tester/testLog.cpp > CMakeFiles/testLog.dir/tester/testLog.cpp.i

CMakeFiles/testLog.dir/tester/testLog.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/testLog.dir/tester/testLog.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/luo/cplus/gaiya/tester/testLog.cpp -o CMakeFiles/testLog.dir/tester/testLog.cpp.s

# Object files for target testLog
testLog_OBJECTS = \
"CMakeFiles/testLog.dir/tester/testLog.cpp.o"

# External object files for target testLog
testLog_EXTERNAL_OBJECTS =

/home/luo/cplus/gaiya/bin/testLog: CMakeFiles/testLog.dir/tester/testLog.cpp.o
/home/luo/cplus/gaiya/bin/testLog: CMakeFiles/testLog.dir/build.make
/home/luo/cplus/gaiya/bin/testLog: /home/luo/cplus/gaiya/lib/libgaiya.so
/home/luo/cplus/gaiya/bin/testLog: CMakeFiles/testLog.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/luo/cplus/gaiya/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/luo/cplus/gaiya/bin/testLog"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testLog.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testLog.dir/build: /home/luo/cplus/gaiya/bin/testLog
.PHONY : CMakeFiles/testLog.dir/build

CMakeFiles/testLog.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testLog.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testLog.dir/clean

CMakeFiles/testLog.dir/depend:
	cd /home/luo/cplus/gaiya/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/luo/cplus/gaiya /home/luo/cplus/gaiya /home/luo/cplus/gaiya/build /home/luo/cplus/gaiya/build /home/luo/cplus/gaiya/build/CMakeFiles/testLog.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/testLog.dir/depend

