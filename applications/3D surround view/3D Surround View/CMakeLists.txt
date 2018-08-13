cmake_minimum_required(VERSION 2.8)

project(surround_view)

find_package(OpenCV 2.4 REQUIRED)
#find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

IF (${OPENGL_FOUND})
    MESSAGE(STATUS "Found OpenGL....")
ELSE (${OPENGL_FOUND})
	MESSAGE(ERROR "OpenGL not found...")
ENDIF (${OPENGL_FOUND})

IF (${GLUT_FOUND})
    MESSAGE(STATUS "Found GLUT....")
ENDIF (${GLUT_FOUND})

IF (${GLEW_FOUND})
    MESSAGE(STATUS "Found GLEW....")
ENDIF (${GLEW_FOUND})

IF (${glfw3_FOUND})
    MESSAGE(STATUS "Found glfw3....")
ENDIF (${glfw3_FOUND})

set (CMAKE_CXX_STANDARD 11)
set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
#optimization level
set(CMAKE_CXX_FLAGS "-Wall -Wextra -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# for debug
set(CMAKE_BUILD_TYPE Debug)

set(OpenCV_INCLUDE_DIRS "/home/nvidia/opencv-2.4.12/include")
include_directories(${OpenCV_INCLUDE_DIRS})
#include_directories(${OpenGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIRS})
include_directories(${glfw3_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories("/home/nvidia/drive-t186ref-linux/include")
include_directories("/home/nvidia/drive-t186ref-linux/samples/nvmedia/MOD/winintf")
include_directories("/home/nvidia/drive-t186ref-linux/samples/nvmedia/3D Surround View")
include_directories("/home/nvidia/drive-t186ref-linux/samples/nvmedia/utils")
include_directories("/home/nvidia/drive-t186ref-linux/lib-target")
link_directories(/home/nvidia/drive-t186ref-linux/lib-target)
link_directories(/home/nvidia/Desktop/glm/glm)

add_executable(surround_view 
    surround_view.cpp 
    shader.cpp 
    texture.cpp 
    objloader.cpp
    utils.cpp
	eglconsumer_main.c
	nvmimg_consumer.c
	eglstrm_setup.c
	../utils/buffer_utils.c
	../utils/config_parser.c
	../utils/log_utils.c
	../utils/misc_utils.c
	../utils/surf_utils.c
	../utils/thread_utils.c
	winintf/egl_utils.c
	winintf/win_x11.c
    )

#MESSAGE(STATUS "OpenCV_LIBS: " ${OpenCV_LIBS})
target_link_libraries(surround_view  ${OpenCV_LIBS})
#target_link_libraries(surround_view  ${OPENGL_gl_LIBRARY})
target_link_libraries(surround_view  ${OPENGL_glu_LIBRARY})
target_link_libraries(surround_view  ${GLEW_LIBRARIES})
target_link_libraries(surround_view  nvmedia)
target_link_libraries(surround_view  EGL)
target_link_libraries(surround_view  X11)
target_link_libraries(surround_view  GLESv2)
target_link_libraries(surround_view  glfw)
target_link_libraries(surround_view  glut)
