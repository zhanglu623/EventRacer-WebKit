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
include online/CMakeFiles/onlinerace.dir/depend.make

# Include the progress variables for this target.
include online/CMakeFiles/onlinerace.dir/progress.make

# Include the compile flags for this target's objects.
include online/CMakeFiles/onlinerace.dir/flags.make

online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o: online/CMakeFiles/onlinerace.dir/flags.make
online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o: /home/jack/src/EventRacer/src/online/VCRaceDetector.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jack/src/EventRacer/bin/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o"
	cd /home/jack/src/EventRacer/bin/online && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o -c /home/jack/src/EventRacer/src/online/VCRaceDetector.cpp

online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.i"
	cd /home/jack/src/EventRacer/bin/online && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/jack/src/EventRacer/src/online/VCRaceDetector.cpp > CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.i

online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.s"
	cd /home/jack/src/EventRacer/bin/online && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/jack/src/EventRacer/src/online/VCRaceDetector.cpp -o CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.s

online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o.requires:
.PHONY : online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o.requires

online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o.provides: online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o.requires
	$(MAKE) -f online/CMakeFiles/onlinerace.dir/build.make online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o.provides.build
.PHONY : online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o.provides

online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o.provides.build: online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o

# Object files for target onlinerace
onlinerace_OBJECTS = \
"CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o"

# External object files for target onlinerace
onlinerace_EXTERNAL_OBJECTS =

online/libonlinerace.a: online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o
online/libonlinerace.a: online/CMakeFiles/onlinerace.dir/build.make
online/libonlinerace.a: online/CMakeFiles/onlinerace.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library libonlinerace.a"
	cd /home/jack/src/EventRacer/bin/online && $(CMAKE_COMMAND) -P CMakeFiles/onlinerace.dir/cmake_clean_target.cmake
	cd /home/jack/src/EventRacer/bin/online && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/onlinerace.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
online/CMakeFiles/onlinerace.dir/build: online/libonlinerace.a
.PHONY : online/CMakeFiles/onlinerace.dir/build

online/CMakeFiles/onlinerace.dir/requires: online/CMakeFiles/onlinerace.dir/VCRaceDetector.cpp.o.requires
.PHONY : online/CMakeFiles/onlinerace.dir/requires

online/CMakeFiles/onlinerace.dir/clean:
	cd /home/jack/src/EventRacer/bin/online && $(CMAKE_COMMAND) -P CMakeFiles/onlinerace.dir/cmake_clean.cmake
.PHONY : online/CMakeFiles/onlinerace.dir/clean

online/CMakeFiles/onlinerace.dir/depend:
	cd /home/jack/src/EventRacer/bin && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jack/src/EventRacer/src /home/jack/src/EventRacer/src/online /home/jack/src/EventRacer/bin /home/jack/src/EventRacer/bin/online /home/jack/src/EventRacer/bin/online/CMakeFiles/onlinerace.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : online/CMakeFiles/onlinerace.dir/depend

