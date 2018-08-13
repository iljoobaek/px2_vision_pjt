#### File structures:  
NVIDIA_TX1/  - project workspace  
    .		   - main folder, including CMakeLists.txt, *.cpp and *.h  
    data/    - videos and pictures  
    doc/     - document introduction about MOD algorithm  
    result/  - statics files generated for measuring the performance, including sequential and CUDA version  
    archive/ - folder for unused original files  

#### Main files introduction:  
- opencv_install.sh
	- a bash script to help install opencv 2.4.13 on ubuntu
	- need to disable openGL feature during the installizaton, otherwise there's an unfixed issue

- CMakeLists.txt
	- recommended method to compile OpenCV code is using cmake

- build.sh
	- a bash script that can be used to compile the MotionObjectDetector project, use `build.sh help` to check the usage

- main.cpp
	- main function that excludes the static object detection part
	- support both seq and cuda version for moving object detection
	- some key macro definitions
        - 'savevideo': enable/disable recording the output videos for seq or cuda, files generated at result/ folder
		- 'enableseq': enable/disable seq version
		- 'correctcheck': enable/disable correctness check, a debug option to check if those Mats are the same for seq and cuda version
		- 'imageprocess': enable/disable image process that coverts the recorded video to images in the data folder, only need to be enabled if new video is recorded

- log.h
	- header file to manage the log printf according to the emergency of the logs
	- ERR > MSG > PERF > INFO > DBG

- utils.h
	- utilities header file that includes the definitions of accuracy analysis APIs
	- some key macro definitions:
		- 'perf_measure': enable/disable perf_measure code in main.cpp
    	- 'accuracy_measure': enable/disable accuracy_measure code in main.cpp

- utils.cpp
	- detailed implementation of definitions in utils.h

- main_original.cpp
	- main function that includes both static object and moving object detection parts, currently not used

- opticalflow.cpp
	- original opticalflow.cpp file

- features.cpp
	- original features.cpp file

- morph.cpp
	- original morph.cpp file

#### How to use?
- git clone the repo from github using 'git clone https://github.com/albd/vision_sdk.git'
- install opencv dependency using the 'opencv_install.sh' script if opencv is not installed
- install cmake for compiling opencv if cmake is not installed
- add environment paramerter OpenCV_DIR with your opencv build folder to ./bashrc
  ex)
  $> echo 'export OpenCV_DIR=/home/nvidia/opencv-2.4.12/build' >> ~/.bashrc
  $> source ~/.bashrc
- check those macro definitions in main.cpp and utils.h to see if they are configured properly
- add png image files data/Pics/
- compile and build the project source code using './build.sh build'
- run the project using './MotionObjectDetector'
- clean the project if necessary using './build.sh clean'
