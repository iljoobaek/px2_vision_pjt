cmake_minimum_required(VERSION 2.8)

project(MotionObjectDetector)

find_package(OpenCV 2.4 REQUIRED)

set (CMAKE_CXX_STANDARD 11)
set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
#optimization level
set(CMAKE_CXX_FLAGS "-Wall -Wextra -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# for debug
set(CMAKE_BUILD_TYPE Debug)

set(OpenCV_INCLUDE_DIRS "/home/nvidia/opencv-2.4.12/include")
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories("/home/nvidia/drive-t186ref-linux/include")
include_directories("/home/nvidia/drive-t186ref-linux/samples/nvmedia/MOD/winintf")
include_directories("/home/nvidia/drive-t186ref-linux/samples/nvmedia/utils")
link_directories(/usr/local/cuda-9.0/lib64)
link_directories(/home/nvidia/drive-t186ref-linux/lib-target)

add_executable(MotionObjectDetector 
    main.cpp 
    features.cpp 
    morph.cpp 
    opticalflow.cpp
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
target_link_libraries(MotionObjectDetector ${OpenCV_LIBS})
target_link_libraries(MotionObjectDetector nvmedia)
target_link_libraries(MotionObjectDetector EGL)
target_link_libraries(MotionObjectDetector X11)
