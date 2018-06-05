/* Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __SAVE_H__
#define __SAVE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "cmdline.h"
#include "thread_utils.h"
#include "surf_utils.h"

/* min no. of buffers to be in circulation at any point */
#define SAVE_QUEUE_SIZE                 3
#define SAVE_DEQUEUE_TIMEOUT            1000
#define SAVE_ENQUEUE_TIMEOUT            100

typedef struct {
    NvQueue                     *inputQueue;
    NvQueue                     *outputQueue;
    volatile uint32_t           *quit;
    uint32_t                    saveEnabled;
    uint32_t                    exitedFlag;

    /* save params */
    uint32_t                    rawBytesPerPixel;
    uint32_t                    pixelOrder;
    char                        *saveFilePrefix;
    uint32_t                    numFramesToSave;
    uint32_t                    virtualGroupIndex;

    /* Raw2Rgb conversion params */
    NvQueue                     *conversionQueue;
    NvMediaSurfaceType          surfType;
    uint32_t                    width;
    uint32_t                    height;
} SaveThreadCtx;

typedef struct {
    /* 2D processing */
    NvThread                    *saveThread;
    SaveThreadCtx               threadCtx;
    NvMediaDevice               *device;

    /* General processing params */
    volatile uint32_t           *quit;
    TestArgs                    *testArgs;
} NvSaveContext;

NvMediaStatus
SaveInit(
        NvMainContext *mainCtx);

NvMediaStatus
SaveFini(
        NvMainContext *mainCtx);

NvMediaStatus
SaveProc(
        NvMainContext *mainCtx);

#ifdef __cplusplus
}
#endif

#endif /* __SAVE_H__ */
