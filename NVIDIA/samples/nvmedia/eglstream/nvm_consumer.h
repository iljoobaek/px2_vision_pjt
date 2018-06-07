/*
 * nvm_consumer.h
 *
 * Copyright (c) 2013-2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

//
// DESCRIPTION:   Simple nvmedia consumer header file
//

#ifndef _NVM_CONSUMER_H_
#define _NVM_CONSUMER_H_

#include "eglstrm_setup.h"
#include "nvmedia_core.h"
#include "nvmedia_surface.h"
#include "nvmedia_vmp.h"
#include <nvmedia_eglstream.h>
#include "nvmvid_encode.h"
#include "nvmimg_encode.h"
#include <nvmedia_idp.h>
#include <nvmedia_2d.h>
#include <buffer_utils.h>
#include <misc_utils.h>
#include "egl_utils.h"

typedef struct _test_nvmedia_consumer_display_s
{

    NvMediaDevice *device;
    EGLStreamKHR eglStream;
    EGLDisplay eglDisplay;
    NvMediaEGLStreamConsumer *consumer;
    InputParameters inputParams;
    ImageInputParameters imageInputParams;

    NvMediaVideoOutput *output;
    NvMediaSurfaceType surfaceType;

    NvMediaIDP               *imageDisplay;
    NvMedia2D                *blitter;
    NvMedia2DBlitParameters  *blitParams;
    BufferPool               *outputBuffersPool;

    NvBool    encodeEnable;
    NvBool    metadataEnable;
    NvThread  *procThread;
    NvBool    procThreadExited;
    NvBool    quit;
    volatile NvBool *consumerDone;
    NvBool    yInvert;
} test_nvmedia_consumer_display_s;

/* --- video consumer ---- */
int video_display_init(volatile NvBool *consumerDone,
                       test_nvmedia_consumer_display_s *display,
                       EGLDisplay eglDisplay, EGLStreamKHR eglStream,
                       TestArgs *args);
void video_display_Deinit(test_nvmedia_consumer_display_s *display);

void video_display_Stop(test_nvmedia_consumer_display_s *display);

void video_display_Flush(test_nvmedia_consumer_display_s *display);

/* --- image consumer ---- */
int image_display_init(volatile NvBool *consumerDone,
                       test_nvmedia_consumer_display_s *display,
                       EGLDisplay eglDisplay, EGLStreamKHR eglStream,
                       TestArgs *args);
void image_display_Deinit(test_nvmedia_consumer_display_s *display);

void image_display_Stop(test_nvmedia_consumer_display_s *display);

void image_display_Flush(test_nvmedia_consumer_display_s *display);

#endif //#define _NVM_CONSUMER_H_
