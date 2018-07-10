# Copyright (c) 2015-2017, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

real_dir = ../../../../

include $(real_dir)../../../make/nvdefs.mk
NV_WINSYS = egldevice

TARGETS = nvmipp_raw

CFLAGS   = $(NV_PLATFORM_OPT) $(NV_PLATFORM_CFLAGS) -ggdb
CFLAGS  += -I.
CFLAGS  += -I$(real_dir)../utils
CFLAGS  += -Iwinintf
CFLAGS  += -DMULTI_EGL_STREAM
CPPFLAGS = $(NV_PLATFORM_SDK_INC) $(NV_PLATFORM_CPPFLAGS)
LDFLAGS  = $(NV_PLATFORM_SDK_LIB) $(NV_PLATFORM_TARGET_LIB) $(NV_PLATFORM_LDFLAGS)

OBJS   += eglstrm_setup.o
#OBJS   += gl_consumer.o
OBJS   += ipp_raw.o
OBJS   += ipp_component.o
OBJS   += img_producer.o
#OBJS   += process2d.o
OBJS   += interop.o
OBJS   += cmdline.o
OBJS   += main.o
#OBJS   += winintf/egl_utils.o
OBJS   += $(real_dir)../utils/config_parser.o
OBJS   += $(real_dir)../utils/log_utils.o
OBJS   += $(real_dir)../utils/misc_utils.o
OBJS   += $(real_dir)../utils/thread_utils.o
OBJS   += $(real_dir)../utils/buffer_utils.o

ifeq ($(NV_WINSYS), egldevice)
  #OBJS   += winintf/win_egldevice.o
else ifeq ($(NV_WINSYS), x11)
  OBJS   += winintf/win_x11.o
else ifeq ($(NV_WINSYS), wayland)
  OBJS   += winintf/win_wayland.o
else ifeq ($(NV_WINSYS), screen)
    OBJS   += winintf/win_screen.o
else
  $(error Unrecognized Windowing environment)
endif

LDLIBS  := -L $(real_dir)../utils
LDLIBS  += -lnv_extimgdev
LDLIBS  += -lnv_sampleplugin
LDLIBS  += -lEGL
LDLIBS  += -lGLESv2
LDLIBS  += -lnvmedia
LDLIBS  += -lnvmedia_ipp
LDLIBS  += -lnvmedia_acp
LDLIBS  += -lnvmedia_isc
LDLIBS  += -lnvparser
LDLIBS  += -lnvtvmr
LDLIBS  += -lm

ifeq ($(NV_PLATFORM_OS), Linux)
  LDLIBS  += -lpthread
  LDLIBS  += -ldl
endif

ifeq ($(NV_PLATFORM_OS), QNX)
  CFLAGS += -DNVMEDIA_QNX
  LDLIBS += -lscreen
endif

ifeq ($(NV_WINSYS), x11)
  LDLIBS += -lX11
else ifeq ($(NV_WINSYS), wayland)
  LDLIBS += -lxkbcommon
  LDLIBS += -lwayland-egl
  LDLIBS += -lwayland-client
else ifeq ($(NV_WINSYS), egldevice)
else ifeq ($(NV_WINSYS), screen)
else
  $(error Unrecognized Windowing environment)
endif


default: $(NV_WINSYS)/$(TARGETS)

$(NV_WINSYS):
	mkdir -p $@

$(NV_WINSYS)/$(TARGETS): $(NV_WINSYS) $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean clobber:
	rm -rf $(OBJS) $(NV_WINSYS)/$(TARGETS)
