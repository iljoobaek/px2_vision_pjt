/* Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "cmdline.h"
#include "config_parser.h"
#include "thread_utils.h"
#include "nvmedia_icp.h"
#include "nvmedia_surface.h"
#include "img_dev.h"

/* min no. of buffers needed to capture without any frame drops */
#define CAPTURE_INPUT_QUEUE_SIZE             5

#define CAPTURE_DEQUEUE_TIMEOUT              1000
#define CAPTURE_ENQUEUE_TIMEOUT              100
#define CAPTURE_FEED_FRAME_TIMEOUT           100
#define CAPTURE_GET_FRAME_TIMEOUT            500
#define CAPTURE_MAX_RETRY                    10
/*
 * This application only supports one OV10635 sensor
 * connected on 0th port.
 */
#define CAPTURE_SENSOR_ID                   0
#define CAPTURE_VC_ID                       0
#define CAPTURE_VC_NUMBER                   1
#define CAPTURE_SENSOR_NUMBER               1
#define CAMERA_SUPPORTED                    "ref_max9286_9271_ov10635"

typedef struct {
    NvMediaICPEx                   *icpExCtx;
    NvQueue                        *inputQueue;
    NvQueue                        *outputQueue;
    volatile uint32_t              *quit;
    uint32_t                       exitedFlag;
    NvMediaICPSettings             *settings;

    /* capture params */
    uint32_t                       width;
    uint32_t                       height;
    uint32_t                       virtualGroupIndex;
    uint32_t                       currentFrame;
    uint32_t                       numFramesToCapture;
    uint32_t                       numBuffers;

    /* surface params */
    NvMediaSurfaceType             surfType;
    uint32_t                       rawBytesPerPixel;
    uint32_t                       pixelOrder;
    NvMediaSurfAllocAttr           surfAllocAttrs[8];
    uint32_t                       numSurfAllocAttrs;
    uint32_t                       consumervm;
    NvMediaImage                   *images[CAPTURE_INPUT_QUEUE_SIZE];
} CaptureThreadCtx;

typedef struct {
    /* capture context */
    NvThread                    *captureThread;
    CaptureThreadCtx            threadCtx;
    NvMediaICPEx                *icpExCtx;
    NvMediaICPSettingsEx        icpSettingsEx;
    NvMediaDevice               *device;
    ExtImgDevice                *extImgDevice;
    ExtImgDevParam              extImgDevParam;
    CaptureConfigParams         *captureParams;

    /* General Variables */
    volatile uint32_t           *quit;
    TestArgs                    *testArgs;
} NvCaptureContext;

NvMediaStatus
CaptureInit(
    NvMainContext *mainCtx);

NvMediaStatus
CaptureFini(
    NvMainContext *mainCtx);

NvMediaStatus
CaptureProc(
    NvMainContext *mainCtx);

#ifdef __cplusplus
}
#endif

#endif

