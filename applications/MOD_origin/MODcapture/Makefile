# Copyright (c) 2015-2016, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

include ../../../make/nvdefs.mk

TARGETS = nvmimg_cap
CXX=g++
CC=g++
CXXFLAGS   = $(NV_PLATFORM_OPT) $(NV_PLATFORM_CFLAGS)
CXXFLAGS  += -I.
CXXFLAGS  += -I /home/nvidia/opencv-2.4.12/include/opencv
CXXFLAGS  += -Iimage_encoder
CXXFLAGS  += -Istreaming_engine
CXXFLAGS  += -Istreaming_engine/capture
CXXFLAGS  += -Istreaming_engine/isp
CXXFLAGS  += -I../utils
CPPFLAGS = $(NV_PLATFORM_SDK_INC) $(NV_PLATFORM_CPPFLAGS)
LDFLAGS  = $(NV_PLATFORM_SDK_LIB) $(NV_PLATFORM_TARGET_LIB) $(NV_PLATFORM_LDFLAGS)

OBJS   := capture.o
OBJS   += check_version.o
OBJS   += cmd_handler.o
OBJS   += cmdline.o
OBJS   += composite.o
OBJS   += display.o
OBJS   += err_handler.o
OBJS   += main.o
OBJS   += parser.o
OBJS   += save.o
OBJS   += ../utils/config_parser.o
OBJS   += ../utils/log_utils.o
OBJS   += ../utils/misc_utils.o
OBJS   += /home/nvidia/drive-t186ref-linux/samples/nvmedia/img_capture/surf_utils.o
OBJS   += ../utils/thread_utils.o

LDLIBS  := -L ../utils
LDLIBS  += -lnv_extimgdev
LDLIBS  += -lnvmedia
LDLIBS  += -lnvtvmr
LDLIBS  += -lz
LDLIBS  += -lm
LDLIBS  += -lnvmedia_isc
LDLIBS  += -lopencv_core
LDLIBS  += -lopencv_imgproc
LDLIBS  += -lopencv_flann
LDLIBS  += -lopencv_highgui
LDLIBS  += -lopencv_ml
LDLIBS  += -lopencv_objdetect
LDLIBS  += -lopencv_photo
LDLIBS  += -lopencv_features2d
LDLIBS  += -lopencv_calib3d
LDLIBS  += -lopencv_stitching
LDLIBS  += -lopencv_videostab

CXXFLAGS  += -D_FILE_OFFSET_BITS=64

ifeq ($(NV_PLATFORM_OS), Linux)
    LDLIBS  += -lpthread
endif

ifeq ($(NV_PLATFORM_OS), QNX)
  CXXFLAGS += -DNVMEDIA_QNX
endif

include ../../../make/nvdefs.mk

$(TARGETS): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean clobber:
	rm -rf $(OBJS) $(TARGETS)
