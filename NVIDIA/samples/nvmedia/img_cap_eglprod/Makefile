# Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

real_dir = ../../../../

include $(real_dir)../../../make/nvdefs.mk

TARGETS = nvm_imgcap_eglprod

CFLAGS   = $(NV_PLATFORM_OPT) $(NV_PLATFORM_CFLAGS) 
CFLAGS  += -I.
CFLAGS  += -I$(real_dir)../utils
CPPFLAGS = $(NV_PLATFORM_SDK_INC) $(NV_PLATFORM_CPPFLAGS)
LDFLAGS  = $(NV_PLATFORM_SDK_LIB) $(NV_PLATFORM_TARGET_LIB) $(NV_PLATFORM_LDFLAGS)

OBJS   := capture.o
OBJS   += eglstrm_setup.o
OBJS   += nvmimg_producer.o
OBJS   += check_version.o
OBJS   += cmdline.o
OBJS   += composite.o
OBJS   += err_handler.o
OBJS   += main.o
OBJS   += parser.o
OBJS   += save.o
OBJS   += $(real_dir)../utils/config_parser.o
OBJS   += $(real_dir)../utils/log_utils.o
OBJS   += $(real_dir)../utils/misc_utils.o
OBJS   += $(real_dir)../utils/surf_utils.o
OBJS   += $(real_dir)../utils/thread_utils.o


LDLIBS  := -L $(real_dir)../utils
LDLIBS  += -lnv_extimgdev
LDLIBS  += -lnvmedia
LDLIBS  += -lnvtvmr
LDLIBS  += -lz
LDLIBS  += -lm
LDLIBS  += -lnvmedia_isc
LDLIBS  += -lEGL
LDLIBS  += -lGLESv2

CFLAGS  += -D_FILE_OFFSET_BITS=64
CFLAGS  += -DMULTI_EGL_STREAM

ifeq ($(NV_PLATFORM_OS), Linux)
    LDLIBS  += -lpthread
    LDLIBS  += -ldl
    LDLIBS  += -lrt
endif

ifeq ($(NV_PLATFORM_OS), QNX)
  CFLAGS += -DNVMEDIA_QNX
endif


$(TARGETS): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean clobber:
	rm -rf $(OBJS) $(TARGETS)
