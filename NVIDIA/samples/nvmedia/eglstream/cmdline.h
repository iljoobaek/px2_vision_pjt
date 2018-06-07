/*
 * cmdline.h
 *
 * Copyright (c) 2014-2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

//
// DESCRIPTION:   Command line parsing for the test application
//

#ifndef _NVMEDIA_TEST_CMD_LINE_H_
#define _NVMEDIA_TEST_CMD_LINE_H_

#include <nvcommon.h>
#include "nvmedia_core.h"
#include "nvmedia_surface.h"

#define MAX_IP_LEN       16

#define STANDALONE_NONE        0
#define STANDALONE_PRODUCER    1
#define STANDALONE_CONSUMER    2
#define STANDALONE_CP_PRODUCER 3
#define STANDALONE_CP_CONSUMER 4

// Command line producer/consumer IDs.  Not all producer and consumers are
// available on all platform builds.  Unsupported consumers are skipped to
// keep valid enums packed.
enum {
    VIDEO_PRODUCER=0,
    IMAGE_PRODUCER,
    GL_PRODUCER,
#ifdef CUDA_SUPPORT
    CUDA_PRODUCER,
#endif
    PRODUCER_COUNT,
    DEFAULT_PRODUCER=VIDEO_PRODUCER,
};

enum {
    VIDEO_CONSUMER=0,
    IMAGE_CONSUMER,
    GL_CONSUMER,
#ifdef CUDA_SUPPORT
    CUDA_CONSUMER,
#endif
#ifdef NVMEDIA_QNX
    SCREEN_WINDOW_CONSUMER,
#endif
#ifdef EGLOUTPUT_SUPPORT
    EGLOUTPUT_CONSUMER,
#endif
    CONSUMER_COUNT,
    DEFAULT_CONSUMER=VIDEO_CONSUMER,
};

typedef struct _TestArgs {
    char   *infile;
    char   *outfile;
    int    logLevel;

    NvBool fifoMode;
    NvBool metadata;
    NvBool nvmediaConsumer;
    NvBool nvmediaProducer;
    NvBool nvmediaVideoProducer;
    NvBool nvmediaVideoConsumer;
    NvBool nvmediaImageProducer;
    NvBool nvmediaImageConsumer;
    NvBool nvmediaEncoder;
    NvBool cudaConsumer;
    NvBool cudaProducer;
    NvBool glConsumer;
    NvBool glProducer;
    NvBool screenConsumer;
    NvBool egloutputConsumer;

    NvU32  standalone;
    NvU32  producerType;
    NvU32  crossConsumerType;
    NvU32  consumerVmId;
    NvMediaSurfaceType prodSurfaceType;
    NvMediaSurfaceType consSurfaceType;
    NvBool prodIsRGBA;
    NvBool consIsRGBA;
    NvBool bSemiplanar;
    NvU32  windowId;
    NvU32  displayId;
    NvBool defaultDisplay;
    NvBool displayEnabled;
    NvU32  inputWidth;
    NvU32  inputHeight;
    NvBool pitchLinearOutput;
    NvU32  prodLoop;
    NvU32  prodFrameCount;
    NvBool shaderType;
    int socketport;
    char ip[MAX_IP_LEN];
} TestArgs;

void PrintUsage(void);
int MainParseArgs(int argc, char **argv, TestArgs *args);

#endif
