/*
 * nvmimage_producer.h
 *
 * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

/*
 * DESCRIPTION:   Simple image producer header file
 */

#ifndef __NVMIMAGE_PRODUDER_H__
#define __NVMIMAGE_PRODUDER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include "buffer_utils.h"
#include "surf_utils.h"
#include "log_utils.h"
#include "misc_utils.h"

#include "nvcommon.h"
#include "eglstrm_setup.h"
#include "cmdline.h"
#include "nvmedia_core.h"
#include "nvmedia_surface.h"
#include "nvmedia_image.h"
#include "nvmedia_isp.h"
#include "nvmedia_2d.h"
#include <nvmedia_eglstream.h>
#include "main.h"


#define MAX_CONFIG_SECTIONS             128

#define IMAGE_BUFFERS_POOL_SIZE         4
#define BUFFER_POOL_TIMEOUT             100
#define EGL_QUEUE_SIZE                  10
#define EGL_DEQUEUE_TIMEOUT             1000
#define EGL_ENQUEUE_TIMEOUT             100

typedef struct {
  char      name[MAX_STRING_SIZE];
  char      description[MAX_STRING_SIZE];
  uint32_t  blendFunc;
  uint32_t  alphaMode;
  uint32_t  alphaValue;
  char      srcRect[MAX_STRING_SIZE];
  char      dstRect[MAX_STRING_SIZE];
  uint32_t  colorStandard;
  uint32_t  colorRange;
  char      cscMatrix[MAX_STRING_SIZE];
  uint32_t  stretchFilter;
  uint32_t  srcOverride;
  uint32_t  srcOverrideAlpha;
  uint32_t  dstTransform;
} ProcessorConfig;

typedef struct {

   /* Output-Image Parameters */
    NvMediaSurfaceType          outputSurfType;
    uint32_t                    outputWidth;
    uint32_t                    outputHeight;

   /* Input-Image Parameters */
    NvMediaSurfaceType          inputSurfType;
    uint32_t                    inputWidth;
    uint32_t                    inputHeight;
    bool                        pitchLinearOutput;
    char                        *inputImages;

    uint32_t                    loop;
    uint32_t                    frameCount;
    bool                        directFlip;
    bool                        crossPartProducer;
    bool                        consumerVmId;

    /* Image2D Params */
    NvMediaDevice               *device;
    NvMedia2D                   *blitter;
    NvMedia2DBlitParameters     *blitParams;
    NvMediaRect                 *dstRect;
    NvMediaRect                 *srcRect;

    NvMediaTime                 baseTime;
    double                      frameTimeUSec;
    uint32_t                    lDispCounter;

    /* Buffer-pool */
    BufferPool                  *inputBuffersPool;
    BufferPool                  *outputBuffersPool;

    /* EGL params */
    NvMediaEGLStreamProducer    *producer;
    EGLStreamKHR                eglStream;
    EGLDisplay                  eglDisplay;

    /* Threading */
    NvThread                    *thread;

    bool                        metadataEnable;
    volatile bool               *producerStop;

} Image2DTestArgs;

#if defined(EGL_KHR_stream)
int Image2DInit(volatile NvBool *decodeFinished,
                EGLDisplay eglDisplay,
                EGLStreamKHR eglStream,
                TestArgs *args);
#endif
void Image2DDeinit(void);
void Image2DproducerStop(void);
void Image2DproducerFlush(void);


typedef struct {
    NvQueue                     *inputQueue;
    NvMediaDevice               *device;
    volatile uint32_t           *quit;
    NvThread                    *ProducerThread;
    uint32_t                    exitedFlag;
    EGLStreamKHR                eglStream;
    EGLStreamKHR                eglStream1;
    EGLDisplay                  eglDisplay;
    EGLDisplay                  eglDisplay1;
    EGLint                      streamState;
    NvMediaEGLStreamProducer    *eglProducer;
    NvMediaEGLStreamProducer    *eglProducer1;
} NvEglStreamContext;

NvMediaStatus
EglProducerInit(
        NvMainContext *mainCtx);

void
EglProducerFini(
        NvMainContext *mainCtx);

NvMediaStatus
EglProducerProc(
        NvMainContext *mainCtx);


#ifdef __cplusplus
}
#endif

#endif /* __NVMIMAGE_PRODUDER_H__*/
