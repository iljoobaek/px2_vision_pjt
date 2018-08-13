cmake_minimum_required(VERSION 2.8)

project(MotionObjectDetector)

find_package(OpenCV 2.4 REQUIRED)

set (CMAKE_CXX_STANDARD 11)
set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
#optimization level
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# for debug
set(CMAKE_BUILD_TYPE Debug)

set(OpenCV_INCLUDE_DIRS "/home/nvidia/opencv-2.4.12/include")
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(/usr/local/cuda-9.0/lib64)

add_executable(MotionObjectDetector 
    main.cpp 
    server.cpp
    features.cpp 
    morph.cpp 
    opticalflow.cpp
    utils.cpp
    )

#MESSAGE(STATUS "OpenCV_LIBS: " ${OpenCV_LIBS})
target_link_libraries(MotionObjectDetector ${OpenCV_LIBS})
