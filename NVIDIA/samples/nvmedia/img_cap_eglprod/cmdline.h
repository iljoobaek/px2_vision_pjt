/* Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef _CMDLINE_H_
#define _CMDLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "nvcommon.h"
#include "nvmedia_idp.h"
#include "nvmedia_core.h"
#include "nvmedia_surface.h"
#include "log_utils.h"
#include "misc_utils.h"
#include "config_parser.h"
#include "img_dev.h"

#include "parser.h"

#define MAX_STRING_SIZE             256
#define MAX_IP_LEN                  16
#define STANDALONE_CROSS_PART       0
#define STANDALONE_CROSS_PROC       1
#define EXT_SYNC_DUTY_RATIO         0.25

typedef struct {
    uint32_t                    isUsed;
    union {
        uint32_t                uIntValue;
        float                   floatValue;
        char                    stringValue[MAX_STRING_SIZE];
    };
} CmdlineParameter;

typedef struct {
    CmdlineParameter            filePrefix;
    uint32_t                    logLevel;
    CmdlineParameter            configFile;
    CmdlineParameter            config[NVMEDIA_ICP_MAX_VIRTUAL_GROUPS];
    CmdlineParameter            numFrames;
    CaptureConfigParams         *captureConfigCollection;
    uint32_t                    captureConfigSetsNum;
    uint32_t                    slaveTegra;
    ExtImgDevMapInfo            camMap;
    uint32_t                    enableExtSync;
    float                       dutyRatio;
    uint8_t                     flipY;
    uint32_t                    consumervm;
    uint32_t                    cross;
    int                         socketport;
    char                        ip[MAX_IP_LEN];
    uint32_t                    isRgba;
    NvMediaBool                 useVirtualChannels;
    NvU32                       numSensors;
    NvU32                       numLinks;
    NvU32                       numVirtualChannels;
    NvMediaBool                 useAggregationFlag;
} TestArgs;

NvMediaStatus
ParseArgs(
          int argc,
          char *argv[],
          TestArgs *allArgs);

void
PrintUsage(
        void);

#ifdef __cplusplus
}
#endif

#endif
