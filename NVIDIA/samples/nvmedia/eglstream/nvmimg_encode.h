/*
 * Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include <nvmedia_iep.h>
#include <stdlib.h>
#include <string.h>
#include <buffer_utils.h>
#include "log_utils.h"

typedef struct {
    NvMediaEncodeConfigH264     encodeConfigH264Params;

    //internal variables
    FILE                        *outputFile;
    NvMediaIEP                  *h264Encoder;
    NvU32                       uFrameCounter;
    NvMediaEncodePicParamsH264  encodePicParams;
} ImageInputParameters;

NvMediaStatus EncodeOneImageFrame(ImageInputParameters *pParams,
                                  NvMediaImage *pNvMediaImage,
                                  NvMediaRect *pSourceRect);

NvMediaStatus ImageEncoderInit(ImageInputParameters *pInputParams,
                               int width,
                               int height,
                               NvMediaSurfaceType surfaceType,
                               NvMediaDevice *device);

NvMediaStatus ImageEncoderDeinit(ImageInputParameters *pInputParams);
