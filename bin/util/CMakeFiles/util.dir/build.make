# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_SOURCE_DIR = /home/jack/src/EventRacer/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jack/src/EventRacer/bin

# Include any dependencies generated for this target.
include util/CMakeFiles/util.dir/depend.make

# Include the progress variables for this target.
include util/CMakeFiles/util.dir/progress.make

# Include the compile flags for this target's objects.
include util/CMakeFiles/util.dir/flags.make

util/CMakeFiles/util.dir/Escaping.cpp.o: util/CMakeFiles/util.dir/flags.make
util/CMakeFiles/util.dir/Escaping.cpp.o: /home/jack/src/EventRacer/src/util/Escaping.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jack/src/EventRacer/bin/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object util/CMakeFiles/util.dir/Escaping.cpp.o"
	cd /home/jack/src/EventRacer/bin/util && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/util.dir/Escaping.cpp.o -c /home/jack/src/EventRacer/src/util/Escaping.cpp

util/CMakeFiles/util.dir/Escaping.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/util.dir/Escaping.cpp.i"
	cd /home/jack/src/EventRacer/bin/util && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/jack/src/EventRacer/src/util/Escaping.cpp > CMakeFiles/util.dir/Escaping.cpp.i

util/CMakeFiles/util.dir/Escaping.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/util.dir/Escaping.cpp.s"
	cd /home/jack/src/EventRacer/bin/util && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/jack/src/EventRacer/src/util/Escaping.cpp -o CMakeFiles/util.dir/Escaping.cpp.s

util/CMakeFiles/util.dir/Escaping.cpp.o.requires:
.PHONY : util/CMakeFiles/util.dir/Escaping.cpp.o.requires

util/CMakeFiles/util.dir/Escaping.cpp.o.provides: util/CMakeFiles/util.dir/Escaping.cpp.o.requires
	$(MAKE) -f util/CMakeFiles/util.dir/build.make util/CMakeFiles/util.dir/Escaping.cpp.o.provides.build
.PHONY : util/CMakeFiles/util.dir/Escaping.cpp.o.provides

util/CMakeFiles/util.dir/Escaping.cpp.o.provides.build: util/CMakeFiles/util.dir/Escaping.cpp.o

util/CMakeFiles/util.dir/UrlEncoding.cpp.o: util/CMakeFiles/util.dir/flags.make
util/CMakeFiles/util.dir/UrlEncoding.cpp.o: /home/jack/src/EventRacer/src/util/UrlEncoding.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jack/src/EventRacer/bin/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object util/CMakeFiles/util.dir/UrlEncoding.cpp.o"
	cd /home/jack/src/EventRacer/bin/util && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/util.dir/UrlEncoding.cpp.o -c /home/jack/src/EventRacer/src/util/UrlEncoding.cpp

util/CMakeFiles/util.dir/UrlEncoding.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/util.dir/UrlEncoding.cpp.i"
	cd /home/jack/src/EventRacer/bin/util && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/jack/src/EventRacer/src/util/UrlEncoding.cpp > CMakeFiles/util.dir/UrlEncoding.cpp.i

util/CMakeFiles/util.dir/UrlEncoding.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/util.dir/UrlEncoding.cpp.s"
	cd /home/jack/src/EventRacer/bin/util && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/jack/src/EventRacer/src/util/UrlEncoding.cpp -o CMakeFiles/util.dir/UrlEncoding.cpp.s

util/CMakeFiles/util.dir/UrlEncoding.cpp.o.requires:
.PHONY : util/CMakeFiles/util.dir/UrlEncoding.cpp.o.requires

util/CMakeFiles/util.dir/UrlEncoding.cpp.o.provides: util/CMakeFiles/util.dir/UrlEncoding.cpp.o.requires
	$(MAKE) -f util/CMakeFiles/util.dir/build.make util/CMakeFiles/util.dir/UrlEncoding.cpp.o.provides.build
.PHONY : util/CMakeFiles/util.dir/UrlEncoding.cpp.o.provides

util/CMakeFiles/util.dir/UrlEncoding.cpp.o.provides.build: util/CMakeFiles/util.dir/UrlEncoding.cpp.o

# Object files for target util
util_OBJECTS = \
"CMakeFiles/util.dir/Escaping.cpp.o" \
"CMakeFiles/util.dir/UrlEncoding.cpp.o"

# External object files for target util
util_EXTERNAL_OBJECTS =

util/libutil.a: util/CMakeFiles/util.dir/Escaping.cpp.o
util/libutil.a: util/CMakeFiles/util.dir/UrlEncoding.cpp.o
util/libutil.a: util/CMakeFiles/util.dir/build.make
util/libutil.a: util/CMakeFiles/util.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library libutil.a"
	cd /home/jack/src/EventRacer/bin/util && $(CMAKE_COMMAND) -P CMakeFiles/util.dir/cmake_clean_target.cmake
	cd /home/jack/src/EventRacer/bin/util && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/util.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
util/CMakeFiles/util.dir/build: util/libutil.a
.PHONY : util/CMakeFiles/util.dir/build

util/CMakeFiles/util.dir/requires: util/CMakeFiles/util.dir/Escaping.cpp.o.requires
util/CMakeFiles/util.dir/requires: util/CMakeFiles/util.dir/UrlEncoding.cpp.o.requires
.PHONY : util/CMakeFiles/util.dir/requires

util/CMakeFiles/util.dir/clean:
	cd /home/jack/src/EventRacer/bin/util && $(CMAKE_COMMAND) -P CMakeFiles/util.dir/cmake_clean.cmake
.PHONY : util/CMakeFiles/util.dir/clean

util/CMakeFiles/util.dir/depend:
	cd /home/jack/src/EventRacer/bin && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jack/src/EventRacer/src /home/jack/src/EventRacer/src/util /home/jack/src/EventRacer/bin /home/jack/src/EventRacer/bin/util /home/jack/src/EventRacer/bin/util/CMakeFiles/util.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : util/CMakeFiles/util.dir/depend

