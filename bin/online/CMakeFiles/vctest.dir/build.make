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
include online/CMakeFiles/vctest.dir/depend.make

# Include the progress variables for this target.
include online/CMakeFiles/vctest.dir/progress.make

# Include the compile flags for this target's objects.
include online/CMakeFiles/vctest.dir/flags.make

online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o: online/CMakeFiles/vctest.dir/flags.make
online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o: /home/jack/src/EventRacer/src/online/VCRaceDetectorTest.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jack/src/EventRacer/bin/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o"
	cd /home/jack/src/EventRacer/bin/online && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o -c /home/jack/src/EventRacer/src/online/VCRaceDetectorTest.cpp

online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.i"
	cd /home/jack/src/EventRacer/bin/online && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/jack/src/EventRacer/src/online/VCRaceDetectorTest.cpp > CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.i

online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.s"
	cd /home/jack/src/EventRacer/bin/online && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/jack/src/EventRacer/src/online/VCRaceDetectorTest.cpp -o CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.s

online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o.requires:
.PHONY : online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o.requires

online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o.provides: online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o.requires
	$(MAKE) -f online/CMakeFiles/vctest.dir/build.make online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o.provides.build
.PHONY : online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o.provides

online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o.provides.build: online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o

# Object files for target vctest
vctest_OBJECTS = \
"CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o"

# External object files for target vctest
vctest_EXTERNAL_OBJECTS =

online/vctest: online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o
online/vctest: online/CMakeFiles/vctest.dir/build.make
online/vctest: online/libonlinerace.a
online/vctest: base/libbase.a
online/vctest: online/CMakeFiles/vctest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable vctest"
	cd /home/jack/src/EventRacer/bin/online && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/vctest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
online/CMakeFiles/vctest.dir/build: online/vctest
.PHONY : online/CMakeFiles/vctest.dir/build

online/CMakeFiles/vctest.dir/requires: online/CMakeFiles/vctest.dir/VCRaceDetectorTest.cpp.o.requires
.PHONY : online/CMakeFiles/vctest.dir/requires

online/CMakeFiles/vctest.dir/clean:
	cd /home/jack/src/EventRacer/bin/online && $(CMAKE_COMMAND) -P CMakeFiles/vctest.dir/cmake_clean.cmake
.PHONY : online/CMakeFiles/vctest.dir/clean

online/CMakeFiles/vctest.dir/depend:
	cd /home/jack/src/EventRacer/bin && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jack/src/EventRacer/src /home/jack/src/EventRacer/src/online /home/jack/src/EventRacer/bin /home/jack/src/EventRacer/bin/online /home/jack/src/EventRacer/bin/online/CMakeFiles/vctest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : online/CMakeFiles/vctest.dir/depend

