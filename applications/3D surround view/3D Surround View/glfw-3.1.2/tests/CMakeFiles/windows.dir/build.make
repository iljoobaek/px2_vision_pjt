# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

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
CMAKE_SOURCE_DIR = "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external"

# Include any dependencies generated for this target.
include glfw-3.1.2/tests/CMakeFiles/windows.dir/depend.make

# Include the progress variables for this target.
include glfw-3.1.2/tests/CMakeFiles/windows.dir/progress.make

# Include the compile flags for this target's objects.
include glfw-3.1.2/tests/CMakeFiles/windows.dir/flags.make

glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o: glfw-3.1.2/tests/CMakeFiles/windows.dir/flags.make
glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o: glfw-3.1.2/tests/windows.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o"
	cd "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests" && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/windows.dir/windows.c.o   -c "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests/windows.c"

glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/windows.dir/windows.c.i"
	cd "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests" && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests/windows.c" > CMakeFiles/windows.dir/windows.c.i

glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/windows.dir/windows.c.s"
	cd "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests" && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests/windows.c" -o CMakeFiles/windows.dir/windows.c.s

glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o.requires:

.PHONY : glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o.requires

glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o.provides: glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o.requires
	$(MAKE) -f glfw-3.1.2/tests/CMakeFiles/windows.dir/build.make glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o.provides.build
.PHONY : glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o.provides

glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o.provides.build: glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o


# Object files for target windows
windows_OBJECTS = \
"CMakeFiles/windows.dir/windows.c.o"

# External object files for target windows
windows_EXTERNAL_OBJECTS =

glfw-3.1.2/tests/windows: glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o
glfw-3.1.2/tests/windows: glfw-3.1.2/tests/CMakeFiles/windows.dir/build.make
glfw-3.1.2/tests/windows: glfw-3.1.2/src/libglfw3.a
glfw-3.1.2/tests/windows: /usr/lib/x86_64-linux-gnu/librt.so
glfw-3.1.2/tests/windows: /usr/lib/x86_64-linux-gnu/libm.so
glfw-3.1.2/tests/windows: /usr/lib/x86_64-linux-gnu/libX11.so
glfw-3.1.2/tests/windows: /usr/lib/x86_64-linux-gnu/libXrandr.so
glfw-3.1.2/tests/windows: /usr/lib/x86_64-linux-gnu/libXinerama.so
glfw-3.1.2/tests/windows: /usr/lib/x86_64-linux-gnu/libXi.so
glfw-3.1.2/tests/windows: /usr/lib/x86_64-linux-gnu/libXxf86vm.so
glfw-3.1.2/tests/windows: /usr/lib/x86_64-linux-gnu/libXcursor.so
glfw-3.1.2/tests/windows: /usr/lib/x86_64-linux-gnu/libGL.so
glfw-3.1.2/tests/windows: glfw-3.1.2/tests/CMakeFiles/windows.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable windows"
	cd "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests" && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/windows.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
glfw-3.1.2/tests/CMakeFiles/windows.dir/build: glfw-3.1.2/tests/windows

.PHONY : glfw-3.1.2/tests/CMakeFiles/windows.dir/build

glfw-3.1.2/tests/CMakeFiles/windows.dir/requires: glfw-3.1.2/tests/CMakeFiles/windows.dir/windows.c.o.requires

.PHONY : glfw-3.1.2/tests/CMakeFiles/windows.dir/requires

glfw-3.1.2/tests/CMakeFiles/windows.dir/clean:
	cd "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests" && $(CMAKE_COMMAND) -P CMakeFiles/windows.dir/cmake_clean.cmake
.PHONY : glfw-3.1.2/tests/CMakeFiles/windows.dir/clean

glfw-3.1.2/tests/CMakeFiles/windows.dir/depend:
	cd "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external" "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests" "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external" "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests" "/mnt/RA 3d view/OpenGLTutorials/ogl-master/external/glfw-3.1.2/tests/CMakeFiles/windows.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : glfw-3.1.2/tests/CMakeFiles/windows.dir/depend

