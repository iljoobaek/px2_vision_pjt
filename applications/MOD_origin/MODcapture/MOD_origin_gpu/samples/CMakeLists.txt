cmake_minimum_required(VERSION 2.8)

project( test )

find_package( OpenCV REQUIRED )

#test.cpp 
add_executable( test 
                pyrlk_optical_flow.cpp 
                )

            target_link_libraries( test ${OpenCV_LIBS} )
