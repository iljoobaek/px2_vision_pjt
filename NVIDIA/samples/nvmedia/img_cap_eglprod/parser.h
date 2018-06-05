/* Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#include "nvcommon.h"
#include "nvmedia_core.h"
#include "nvmedia_surface.h"
#include "nvmedia_image.h"

#include "config_parser.h"
#include "misc_utils.h"
#include "log_utils.h"

#define MAX_CONFIG_SECTIONS             128
#define MAX_STRING_SIZE                 256

typedef struct {
  char      name[MAX_STRING_SIZE];
  char      description[MAX_STRING_SIZE];
  char      board[MAX_STRING_SIZE];
  char      inputDevice[MAX_STRING_SIZE];
  char      inputFormat[MAX_STRING_SIZE];
  char      surfaceFormat[MAX_STRING_SIZE];
  char      resolution[MAX_STRING_SIZE];
  char      interface[MAX_STRING_SIZE];
  uint32_t  i2cDevice;
  uint32_t  csiLanes;
  uint32_t  embeddedDataLinesTop;
  uint32_t  embeddedDataLinesBottom;
  uint32_t  desAddr;
  uint32_t  brdcstSerAddr;
  uint32_t  serAddr[NVMEDIA_MAX_AGGREGATE_IMAGES];
  uint32_t  brdcstSensorAddr;
  uint32_t  sensorAddr[NVMEDIA_MAX_AGGREGATE_IMAGES];
} CaptureConfigParams;

NvMediaStatus
ParseConfigFile(
                char *configFile,
                uint32_t *captureConfigSetsNum,
                CaptureConfigParams **captureConfigCollection);
#endif
