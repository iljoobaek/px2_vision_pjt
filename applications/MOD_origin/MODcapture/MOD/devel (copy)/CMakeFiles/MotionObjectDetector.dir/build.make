# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_SOURCE_DIR = /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel

# Include any dependencies generated for this target.
include CMakeFiles/MotionObjectDetector.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/MotionObjectDetector.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MotionObjectDetector.dir/flags.make

CMakeFiles/MotionObjectDetector.dir/main.cpp.o: CMakeFiles/MotionObjectDetector.dir/flags.make
CMakeFiles/MotionObjectDetector.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/MotionObjectDetector.dir/main.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/MotionObjectDetector.dir/main.cpp.o -c /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/main.cpp

CMakeFiles/MotionObjectDetector.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MotionObjectDetector.dir/main.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/main.cpp > CMakeFiles/MotionObjectDetector.dir/main.cpp.i

CMakeFiles/MotionObjectDetector.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MotionObjectDetector.dir/main.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/main.cpp -o CMakeFiles/MotionObjectDetector.dir/main.cpp.s

CMakeFiles/MotionObjectDetector.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/MotionObjectDetector.dir/main.cpp.o.requires

CMakeFiles/MotionObjectDetector.dir/main.cpp.o.provides: CMakeFiles/MotionObjectDetector.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/MotionObjectDetector.dir/build.make CMakeFiles/MotionObjectDetector.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/MotionObjectDetector.dir/main.cpp.o.provides

CMakeFiles/MotionObjectDetector.dir/main.cpp.o.provides.build: CMakeFiles/MotionObjectDetector.dir/main.cpp.o


CMakeFiles/MotionObjectDetector.dir/server.cpp.o: CMakeFiles/MotionObjectDetector.dir/flags.make
CMakeFiles/MotionObjectDetector.dir/server.cpp.o: ../server.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/MotionObjectDetector.dir/server.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/MotionObjectDetector.dir/server.cpp.o -c /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/server.cpp

CMakeFiles/MotionObjectDetector.dir/server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MotionObjectDetector.dir/server.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/server.cpp > CMakeFiles/MotionObjectDetector.dir/server.cpp.i

CMakeFiles/MotionObjectDetector.dir/server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MotionObjectDetector.dir/server.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/server.cpp -o CMakeFiles/MotionObjectDetector.dir/server.cpp.s

CMakeFiles/MotionObjectDetector.dir/server.cpp.o.requires:

.PHONY : CMakeFiles/MotionObjectDetector.dir/server.cpp.o.requires

CMakeFiles/MotionObjectDetector.dir/server.cpp.o.provides: CMakeFiles/MotionObjectDetector.dir/server.cpp.o.requires
	$(MAKE) -f CMakeFiles/MotionObjectDetector.dir/build.make CMakeFiles/MotionObjectDetector.dir/server.cpp.o.provides.build
.PHONY : CMakeFiles/MotionObjectDetector.dir/server.cpp.o.provides

CMakeFiles/MotionObjectDetector.dir/server.cpp.o.provides.build: CMakeFiles/MotionObjectDetector.dir/server.cpp.o


CMakeFiles/MotionObjectDetector.dir/features.cpp.o: CMakeFiles/MotionObjectDetector.dir/flags.make
CMakeFiles/MotionObjectDetector.dir/features.cpp.o: ../features.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/MotionObjectDetector.dir/features.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/MotionObjectDetector.dir/features.cpp.o -c /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/features.cpp

CMakeFiles/MotionObjectDetector.dir/features.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MotionObjectDetector.dir/features.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/features.cpp > CMakeFiles/MotionObjectDetector.dir/features.cpp.i

CMakeFiles/MotionObjectDetector.dir/features.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MotionObjectDetector.dir/features.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/features.cpp -o CMakeFiles/MotionObjectDetector.dir/features.cpp.s

CMakeFiles/MotionObjectDetector.dir/features.cpp.o.requires:

.PHONY : CMakeFiles/MotionObjectDetector.dir/features.cpp.o.requires

CMakeFiles/MotionObjectDetector.dir/features.cpp.o.provides: CMakeFiles/MotionObjectDetector.dir/features.cpp.o.requires
	$(MAKE) -f CMakeFiles/MotionObjectDetector.dir/build.make CMakeFiles/MotionObjectDetector.dir/features.cpp.o.provides.build
.PHONY : CMakeFiles/MotionObjectDetector.dir/features.cpp.o.provides

CMakeFiles/MotionObjectDetector.dir/features.cpp.o.provides.build: CMakeFiles/MotionObjectDetector.dir/features.cpp.o


CMakeFiles/MotionObjectDetector.dir/morph.cpp.o: CMakeFiles/MotionObjectDetector.dir/flags.make
CMakeFiles/MotionObjectDetector.dir/morph.cpp.o: ../morph.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/MotionObjectDetector.dir/morph.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/MotionObjectDetector.dir/morph.cpp.o -c /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/morph.cpp

CMakeFiles/MotionObjectDetector.dir/morph.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MotionObjectDetector.dir/morph.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/morph.cpp > CMakeFiles/MotionObjectDetector.dir/morph.cpp.i

CMakeFiles/MotionObjectDetector.dir/morph.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MotionObjectDetector.dir/morph.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/morph.cpp -o CMakeFiles/MotionObjectDetector.dir/morph.cpp.s

CMakeFiles/MotionObjectDetector.dir/morph.cpp.o.requires:

.PHONY : CMakeFiles/MotionObjectDetector.dir/morph.cpp.o.requires

CMakeFiles/MotionObjectDetector.dir/morph.cpp.o.provides: CMakeFiles/MotionObjectDetector.dir/morph.cpp.o.requires
	$(MAKE) -f CMakeFiles/MotionObjectDetector.dir/build.make CMakeFiles/MotionObjectDetector.dir/morph.cpp.o.provides.build
.PHONY : CMakeFiles/MotionObjectDetector.dir/morph.cpp.o.provides

CMakeFiles/MotionObjectDetector.dir/morph.cpp.o.provides.build: CMakeFiles/MotionObjectDetector.dir/morph.cpp.o


CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o: CMakeFiles/MotionObjectDetector.dir/flags.make
CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o: ../opticalflow.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o -c /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/opticalflow.cpp

CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/opticalflow.cpp > CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.i

CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/opticalflow.cpp -o CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.s

CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o.requires:

.PHONY : CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o.requires

CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o.provides: CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o.requires
	$(MAKE) -f CMakeFiles/MotionObjectDetector.dir/build.make CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o.provides.build
.PHONY : CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o.provides

CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o.provides.build: CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o


CMakeFiles/MotionObjectDetector.dir/utils.cpp.o: CMakeFiles/MotionObjectDetector.dir/flags.make
CMakeFiles/MotionObjectDetector.dir/utils.cpp.o: ../utils.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/MotionObjectDetector.dir/utils.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/MotionObjectDetector.dir/utils.cpp.o -c /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/utils.cpp

CMakeFiles/MotionObjectDetector.dir/utils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MotionObjectDetector.dir/utils.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/utils.cpp > CMakeFiles/MotionObjectDetector.dir/utils.cpp.i

CMakeFiles/MotionObjectDetector.dir/utils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MotionObjectDetector.dir/utils.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/utils.cpp -o CMakeFiles/MotionObjectDetector.dir/utils.cpp.s

CMakeFiles/MotionObjectDetector.dir/utils.cpp.o.requires:

.PHONY : CMakeFiles/MotionObjectDetector.dir/utils.cpp.o.requires

CMakeFiles/MotionObjectDetector.dir/utils.cpp.o.provides: CMakeFiles/MotionObjectDetector.dir/utils.cpp.o.requires
	$(MAKE) -f CMakeFiles/MotionObjectDetector.dir/build.make CMakeFiles/MotionObjectDetector.dir/utils.cpp.o.provides.build
.PHONY : CMakeFiles/MotionObjectDetector.dir/utils.cpp.o.provides

CMakeFiles/MotionObjectDetector.dir/utils.cpp.o.provides.build: CMakeFiles/MotionObjectDetector.dir/utils.cpp.o


# Object files for target MotionObjectDetector
MotionObjectDetector_OBJECTS = \
"CMakeFiles/MotionObjectDetector.dir/main.cpp.o" \
"CMakeFiles/MotionObjectDetector.dir/server.cpp.o" \
"CMakeFiles/MotionObjectDetector.dir/features.cpp.o" \
"CMakeFiles/MotionObjectDetector.dir/morph.cpp.o" \
"CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o" \
"CMakeFiles/MotionObjectDetector.dir/utils.cpp.o"

# External object files for target MotionObjectDetector
MotionObjectDetector_EXTERNAL_OBJECTS =

MotionObjectDetector: CMakeFiles/MotionObjectDetector.dir/main.cpp.o
MotionObjectDetector: CMakeFiles/MotionObjectDetector.dir/server.cpp.o
MotionObjectDetector: CMakeFiles/MotionObjectDetector.dir/features.cpp.o
MotionObjectDetector: CMakeFiles/MotionObjectDetector.dir/morph.cpp.o
MotionObjectDetector: CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o
MotionObjectDetector: CMakeFiles/MotionObjectDetector.dir/utils.cpp.o
MotionObjectDetector: CMakeFiles/MotionObjectDetector.dir/build.make
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_videostab.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_ts.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_superres.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_stitching.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_ocl.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_gpu.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_contrib.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_photo.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_legacy.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_video.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_objdetect.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_ml.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_calib3d.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_features2d.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_highgui.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_imgproc.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_flann.so.2.4.9
MotionObjectDetector: /usr/lib/aarch64-linux-gnu/libopencv_core.so.2.4.9
MotionObjectDetector: CMakeFiles/MotionObjectDetector.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX executable MotionObjectDetector"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MotionObjectDetector.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/MotionObjectDetector.dir/build: MotionObjectDetector

.PHONY : CMakeFiles/MotionObjectDetector.dir/build

CMakeFiles/MotionObjectDetector.dir/requires: CMakeFiles/MotionObjectDetector.dir/main.cpp.o.requires
CMakeFiles/MotionObjectDetector.dir/requires: CMakeFiles/MotionObjectDetector.dir/server.cpp.o.requires
CMakeFiles/MotionObjectDetector.dir/requires: CMakeFiles/MotionObjectDetector.dir/features.cpp.o.requires
CMakeFiles/MotionObjectDetector.dir/requires: CMakeFiles/MotionObjectDetector.dir/morph.cpp.o.requires
CMakeFiles/MotionObjectDetector.dir/requires: CMakeFiles/MotionObjectDetector.dir/opticalflow.cpp.o.requires
CMakeFiles/MotionObjectDetector.dir/requires: CMakeFiles/MotionObjectDetector.dir/utils.cpp.o.requires

.PHONY : CMakeFiles/MotionObjectDetector.dir/requires

CMakeFiles/MotionObjectDetector.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MotionObjectDetector.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MotionObjectDetector.dir/clean

CMakeFiles/MotionObjectDetector.dir/depend:
	cd /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel /home/nvidia/drive-t186ref-linux/samples/nvmedia/MODcapture/MOD/devel/CMakeFiles/MotionObjectDetector.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/MotionObjectDetector.dir/depend

