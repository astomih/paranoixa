# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/runner/work/paranoixa/paranoixa

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/runner/work/paranoixa/paranoixa/build/emscripten

# Include any dependencies generated for this target.
include test/CMakeFiles/paranoixa_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/paranoixa_test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/paranoixa_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/paranoixa_test.dir/flags.make

test/CMakeFiles/paranoixa_test.dir/codegen:
.PHONY : test/CMakeFiles/paranoixa_test.dir/codegen

test/CMakeFiles/paranoixa_test.dir/test.cpp.o: test/CMakeFiles/paranoixa_test.dir/flags.make
test/CMakeFiles/paranoixa_test.dir/test.cpp.o: test/CMakeFiles/paranoixa_test.dir/includes_CXX.rsp
test/CMakeFiles/paranoixa_test.dir/test.cpp.o: /home/runner/work/paranoixa/paranoixa/test/test.cpp
test/CMakeFiles/paranoixa_test.dir/test.cpp.o: test/CMakeFiles/paranoixa_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/runner/work/paranoixa/paranoixa/build/emscripten/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/paranoixa_test.dir/test.cpp.o"
	cd /home/runner/work/paranoixa/paranoixa/build/emscripten/test && /home/runner/work/_temp/8218185b-2530-4329-89bd-0a7251916862/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/paranoixa_test.dir/test.cpp.o -MF CMakeFiles/paranoixa_test.dir/test.cpp.o.d -o CMakeFiles/paranoixa_test.dir/test.cpp.o -c /home/runner/work/paranoixa/paranoixa/test/test.cpp

test/CMakeFiles/paranoixa_test.dir/test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/paranoixa_test.dir/test.cpp.i"
	cd /home/runner/work/paranoixa/paranoixa/build/emscripten/test && /home/runner/work/_temp/8218185b-2530-4329-89bd-0a7251916862/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/runner/work/paranoixa/paranoixa/test/test.cpp > CMakeFiles/paranoixa_test.dir/test.cpp.i

test/CMakeFiles/paranoixa_test.dir/test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/paranoixa_test.dir/test.cpp.s"
	cd /home/runner/work/paranoixa/paranoixa/build/emscripten/test && /home/runner/work/_temp/8218185b-2530-4329-89bd-0a7251916862/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/runner/work/paranoixa/paranoixa/test/test.cpp -o CMakeFiles/paranoixa_test.dir/test.cpp.s

# Object files for target paranoixa_test
paranoixa_test_OBJECTS = \
"CMakeFiles/paranoixa_test.dir/test.cpp.o"

# External object files for target paranoixa_test
paranoixa_test_EXTERNAL_OBJECTS =

test/paranoixa_test.html: test/CMakeFiles/paranoixa_test.dir/test.cpp.o
test/paranoixa_test.html: test/CMakeFiles/paranoixa_test.dir/build.make
test/paranoixa_test.html: libparanoixa.a
test/paranoixa_test.html: source/imgui_backend/libparanoixa_imgui_backend.a
test/paranoixa_test.html: libparanoixa.a
test/paranoixa_test.html: library/SDL/libSDL3.a
test/paranoixa_test.html: test/CMakeFiles/paranoixa_test.dir/linkLibs.rsp
test/paranoixa_test.html: test/CMakeFiles/paranoixa_test.dir/objects1.rsp
test/paranoixa_test.html: test/CMakeFiles/paranoixa_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/runner/work/paranoixa/paranoixa/build/emscripten/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable paranoixa_test.html"
	cd /home/runner/work/paranoixa/paranoixa/build/emscripten/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/paranoixa_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/paranoixa_test.dir/build: test/paranoixa_test.html
.PHONY : test/CMakeFiles/paranoixa_test.dir/build

test/CMakeFiles/paranoixa_test.dir/clean:
	cd /home/runner/work/paranoixa/paranoixa/build/emscripten/test && $(CMAKE_COMMAND) -P CMakeFiles/paranoixa_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/paranoixa_test.dir/clean

test/CMakeFiles/paranoixa_test.dir/depend:
	cd /home/runner/work/paranoixa/paranoixa/build/emscripten && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/runner/work/paranoixa/paranoixa /home/runner/work/paranoixa/paranoixa/test /home/runner/work/paranoixa/paranoixa/build/emscripten /home/runner/work/paranoixa/paranoixa/build/emscripten/test /home/runner/work/paranoixa/paranoixa/build/emscripten/test/CMakeFiles/paranoixa_test.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : test/CMakeFiles/paranoixa_test.dir/depend

