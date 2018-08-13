#!/bin/bash

#parameters definition
CMD=$0
ACTION=$1
DEVEL_DIR=devel
BIN_FILE=MotionObjectDetector

mod_build()
{
    if [ -d "$DEVEL_DIR" ];then
        echo "$DEVEL_DIR exists"
    else
        echo "$DEVEL_DIR doesn't exist"
        mkdir -p $DEVEL_DIR
    fi

    cd $DEVEL_DIR
    cmake -D CUDA_USE_STATIC_CUDA_RUNTIME=OFF WITH_CUDA=ON ../
    make
    cp $BIN_FILE ../
    cd ../
    echo -e "build finished successfully!"
}

mod_clean()
{
    rm -rf $DEVEL_DIR
    rm -rf *.o
    rm -rf $BIN_FILE
}

usage()
{
    echo "build command  -- build.sh command"
    echo "build help   --  ./build.sh help"
    echo "build build  --  ./build.sh build"
    echo "build clean  --  ./build.sh clean"
}

main()
{
    case $ACTION in
        "build")
            mod_build;;
        "clean")
            mod_clean;;
        "help")
            usage;;
        *)
            usage;;
    esac
}

main
