/* Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "cmdline.h"

static void
_PrintCaptureParamSets(
            TestArgs *args)
{
    uint32_t j;

    LOG_MSG("Capture parameter sets (%d):\n", args->captureConfigSetsNum);
    for (j = 0; j < args->captureConfigSetsNum; j++) {
        LOG_MSG("\n%s: ", args->captureConfigCollection[j].name);
        LOG_MSG("%s\n", args->captureConfigCollection[j].description);
    }
    LOG_MSG("\n");
}

static int
_GetParamSetID(
               void *configSetsList,
               int paramSets,
               char* paramSetName)
{
    int i;
    char *name = NULL;

    for (i = 0; i < paramSets; i++) {
        name = ((CaptureConfigParams *)configSetsList)[i].name;

        if (!strcasecmp(name, paramSetName)) {
            LOG_DBG("%s: Param set found (%d)\n", __func__, i);
            return i;
        }
    }
    return -1;
}

void
PrintUsage(
        void)
{
    NvMediaIDPDeviceParams outputs[MAX_OUTPUT_DEVICES];
    int outputDevicesNum = 0;

    NvMediaIDPQuery(&outputDevicesNum, outputs);

    LOG_MSG("Usage: nvm_imgcap_eglprod [options]\n");
    LOG_MSG("\nAvailable command line options:\n");
    LOG_MSG("-h                Print usage\n");
    LOG_MSG("-v [level]        Logging Level. Default = 0\n");
    LOG_MSG("                  0: Errors, 1: Warnings, 2: Info, 3: Debug\n");
    LOG_MSG("                  Default: 0\n");
    LOG_MSG("-n [frames]       Number of frames to capture and quit automatically \n");
    LOG_MSG("-f [file-prefix]  Save frames to file.\n");
    LOG_MSG("                  Provide filename pre-fix for each saved file.\n");
    LOG_MSG("-cf [file]        Set configuration file.\n");
    LOG_MSG("                  Default: configs/default.conf\n");
    LOG_MSG("-c [name]         Parameters set name to be used for capture configuration.\n");
    LOG_MSG("                  Only Supported configuration is ref_max9286_9271_ov10635\n");
    LOG_MSG("-lc               List all available config sets.\n");
    LOG_MSG("-cross            set 0 for cross-partition producer\n");
    LOG_MSG("                  set 1 for cross-process producer\n");
    LOG_MSG("                  default = 0\n");
    LOG_MSG("-consumervm [n]   set consumer VM ID for cross-partition producer\n");
    LOG_MSG("-ip               set ip of consumer partition for cross partition"\
                                                        "eglstream communication\n");
    LOG_MSG("-socketport       set port for cross partition eglstream"\
                                "communication[1024-49151]. Default: 8888\n");
    LOG_MSG("-flip             Mirrors the image vertically\n");
    LOG_MSG("-ot [type]        Posted surface type: yuv422p/rgba\n");
    LOG_MSG("\nAvailable run-time commands:\n");
    LOG_MSG("h                 Print available commands\n");
    LOG_MSG("q                 Quit the application\n");
}

NvMediaStatus
ParseArgs(
          int argc,
          char *argv[],
          TestArgs *allArgs)
{
    int i = 0;
    uint32_t bLastArg = NVMEDIA_FALSE;
    uint32_t bDataAvailable = NVMEDIA_FALSE;

    /* Default parameters */
    // krammer add
    allArgs->numSensors = 1;
    allArgs->numVirtualChannels = 1;
    allArgs->camMap.enable = CAM_ENABLE_DEFAULT;
    allArgs->camMap.mask   = CAM_MASK_DEFAULT;
    allArgs->camMap.csiOut = CSI_OUT_DEFAULT;
    allArgs->socketport = 8888;
    allArgs->enableExtSync = NVMEDIA_FALSE;
    allArgs->dutyRatio = EXT_SYNC_DUTY_RATIO;
    allArgs->flipY = NVMEDIA_FALSE;
    allArgs->consumervm = -1;
    allArgs->cross = STANDALONE_CROSS_PART;
    allArgs->isRgba = NVMEDIA_TRUE;
    allArgs->slaveTegra = NVMEDIA_FALSE;
    strncpy(allArgs->ip, "127.0.0.1", 16);

    if (argc < 2) {
        PrintUsage();
        return NVMEDIA_STATUS_ERROR;
    }

    if (argc >= 2) {
        for (i = 1; i < argc; i++) {
            /* Check if this is the last argument */
            bLastArg = ((argc - i) == 1);

            /* Check if there is data available to be parsed */
            bDataAvailable = (!bLastArg) && !(argv[i+1][0] == '-');

            if (!strcasecmp(argv[i], "-h")) {
                PrintUsage();
                return NVMEDIA_STATUS_ERROR;
            } else if (!strcasecmp(argv[i], "-v")) {
                allArgs->logLevel = LEVEL_DBG;
                if (bDataAvailable) {
                    allArgs->logLevel = atoi(argv[++i]);
                    if (allArgs->logLevel > LEVEL_DBG) {
                        printf("Invalid logging level chosen (%d)\n",
                               allArgs->logLevel);
                        printf("Setting logging level to LEVEL_ERR (0)\n");
                        allArgs->logLevel = LEVEL_ERR;
                    }
                }
                SetLogLevel((enum LogLevel)allArgs->logLevel);
            } else if (!strcasecmp(argv[i], "--aggregate")) {
                allArgs->useAggregationFlag = NVMEDIA_TRUE;
                if (bDataAvailable) {
                    if ((sscanf(argv[++i], "%u", &allArgs->numSensors) != 1)) {
                        LOG_ERR("Bad siblings number: %s\n", argv[i]);
                        return NVMEDIA_STATUS_ERROR;
                    }   
                } else {
                    LOG_ERR("--aggregate must be followed by number of images to aggregate\n");
                    return NVMEDIA_STATUS_ERROR;
                }
            } else if (!strcasecmp(argv[i], "-cf")) {
                if (argv[i + 1] && argv[i + 1][0] != '-') {
                    strncpy(allArgs->configFile.stringValue, argv[++i],
                            MAX_STRING_SIZE);
                    allArgs->configFile.isUsed = NVMEDIA_TRUE;
                } else {
                    LOG_ERR("-cf must be followed by configuration file name\n");
                    return NVMEDIA_STATUS_ERROR;
                }
            }
        }
    }

    /* Default config file */
    if (!allArgs->configFile.isUsed) {
        strcpy(allArgs->configFile.stringValue,
               "configs/default.conf");
        allArgs->configFile.isUsed = NVMEDIA_TRUE;
    }

    /* Parse config file here */
    if (IsFailed(ParseConfigFile(allArgs->configFile.stringValue,
                                &allArgs->captureConfigSetsNum,
                                &allArgs->captureConfigCollection))) {
        LOG_ERR("Failed to parse config file %s\n",
                allArgs->configFile.stringValue);
        return NVMEDIA_STATUS_ERROR;
    }

    if (argc >= 2) {
        for (i = 1; i < argc; i++) {
            /* Check if this is the last argument */
            bLastArg = ((argc - i) == 1);

            /* Check if there is data available to be parsed */
            bDataAvailable = (!bLastArg) && !(argv[i+1][0] == '-');

            if (!strcasecmp(argv[i], "-h")) {
                PrintUsage();
                return NVMEDIA_STATUS_ERROR;
            } else if (!strcasecmp(argv[i], "-v")) {
                if (bDataAvailable)
                    ++i;
            } else if (!strcasecmp(argv[i], "-cf")) {
                ++i; /* Was already parsed at the beginning. Skipping.*/
            } else if (!strcasecmp(argv[i], "-lc")) {
                _PrintCaptureParamSets(allArgs);
                return NVMEDIA_STATUS_ERROR;
            } else if (!strcasecmp(argv[i], "-f")) {
                if (argv[i + 1] && argv[i + 1][0] != '-') {
                    strncpy(allArgs->filePrefix.stringValue, argv[++i],
                            MAX_STRING_SIZE);
                } else {
                    LOG_ERR("-f must be followed by a file prefix string\n");
                    return NVMEDIA_STATUS_ERROR;
                }
                allArgs->filePrefix.isUsed = NVMEDIA_TRUE;
            } else if (!strcasecmp(argv[i], "-c")) {
                if (bDataAvailable) {
                    ++i;
                    int paramSetId = 0;
                    paramSetId = _GetParamSetID(allArgs->captureConfigCollection,
                                               allArgs->captureConfigSetsNum,
                                               argv[i]);
                    if (paramSetId == -1) {
                        LOG_ERR("Params set name '%s' wasn't found\n",
                                argv[i]);
                        return NVMEDIA_STATUS_ERROR;
                    }
                    allArgs->config[0].isUsed = NVMEDIA_TRUE;
                    allArgs->config[0].uIntValue = paramSetId;
                    LOG_INFO("Using params set: %s for capture\n",
                             allArgs->captureConfigCollection[paramSetId].name);
                } else {
                    LOG_ERR("-c must be followed by capture parameters set name\n");
                    return NVMEDIA_STATUS_ERROR;
                }
            } else if (!strcasecmp(argv[i], "-n")) {
                if (bDataAvailable) {
                    char *arg = argv[++i];
                    allArgs->numFrames.uIntValue = atoi(arg);
                    allArgs->numFrames.isUsed = NVMEDIA_TRUE;
                } else {
                    LOG_ERR("-n must be followed by number of frames to capture\n");
                    return NVMEDIA_STATUS_ERROR;
                }
            } else if(!strcasecmp(argv[i], "-ot")) {
                ++i;
                if(!strcasecmp(argv[i], "rgba")) {
                    allArgs->isRgba = NVMEDIA_TRUE;
                } else if(!strcasecmp(argv[i], "yuv422p")){
                    allArgs->isRgba = NVMEDIA_FALSE;
                } else {
                    LOG_ERR("-ot must be followed by supported color format\n");
                    LOG_ERR("Taking default value as rgba\n");
                    allArgs->isRgba = NVMEDIA_TRUE;
                }
            } else if (!strcasecmp(argv[i], "-flip")) {
                allArgs->flipY = NV_TRUE;
            } else if (!strcmp(argv[i], "-cross")) {
                if(!bDataAvailable || !sscanf(argv[++i], "%u", &allArgs->cross) ||
                                    (allArgs->cross > 1)) {
                    LOG_ERR("ERR: -cross must be followed by mode [0-1].\n");
                    return 0;
                }
            } else if (!strcasecmp(argv[i], "-ip")) {
                if (!bDataAvailable || !strncpy(allArgs->ip, argv[++i], 16)) {
                    LOG_ERR("ERR: -ip must be followed by a valid IP like 12.0.0.1\n");
                    return 0;
                }
            } else if (!strcasecmp(argv[i], "-socketport")) {
                if (!bDataAvailable || !sscanf(argv[++i], "%u", &allArgs->socketport)) {
                    LOG_ERR("ERR: -socketport must be followed by port number\n");
                    return 0;
                }
                if ((allArgs->socketport < 1024) || (allArgs->socketport > 49151)) {
                    LOG_ERR("ERR: Invalid socket port\n");
                    return 0;
                }
            } else if (!strcasecmp(argv[i], "-consumervm")) {
                if (!bDataAvailable || !sscanf(argv[++i], "%u", &allArgs->consumervm)) {
                    LOG_ERR("ERR: -consumervm must be followed by vm id\n");
                    return 0;
                }
                if ((allArgs->socketport < 1024) || (allArgs->socketport > 49151)) {
                    LOG_ERR("ERR: Invalid socket port\n");
                    return 0;
                }
            } else {
                LOG_ERR("Unsupported option encountered: %s\n", argv[i]);
                return NVMEDIA_STATUS_ERROR;
            }
        }
    }

    if (allArgs->cross == STANDALONE_CROSS_PART) {
        if(!strcmp(allArgs->ip, "127.0.0.1")) {
            LOG_ERR("ERR: -ip must be valid IP like 12.0.0.1 for cross-partition producer\n");
            return 0;
        }
        if(NVMEDIA_FALSE == allArgs->consumervm) {
            LOG_ERR("ERR: -consumervm must be specified for cross-partition producer\n");
            return 0;
        }
    }

    return NVMEDIA_STATUS_OK;
}
