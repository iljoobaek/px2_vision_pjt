/*
 * cmdline.c
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

#include <stdlib.h>
#include <string.h>
#include <cmdline.h>
#include <misc_utils.h>
#include <log_utils.h>

void PrintUsage(void)
{
    NvMediaVideoOutputDeviceParams videoOutputs[MAX_OUTPUT_DEVICES];
    NvMediaStatus rt;
    int outputDevicesNum, i;

    LOG_MSG("Usage:\n");
    LOG_MSG("-h                         Print this usage\n");

    LOG_MSG("\nEGL-VideoProducer Params:\n");
    LOG_MSG("-f [file name]             Input File Name\n");
    LOG_MSG("-l [loops]                 Number of loops of playback\n");
    LOG_MSG("                           -1 for infinite loops of playback (default: 1)\n");

    LOG_MSG("\nEGL-ImageProducer Params:\n");
    LOG_MSG("-f [file]                  Input image file. \n");
    LOG_MSG("                           Input file should in YUV 420 format, UV order\n");
    LOG_MSG("-fr [WxH]                  Input file resolution\n");
    LOG_MSG("-pl                        Producer uses pitchlinear surface.\n");
    LOG_MSG("                           Default uses blocklinear\n");

    LOG_MSG("\n Common-Params:\n");

    LOG_MSG("-v [level]                 Verbose, diagnostics prints\n");
    LOG_MSG("-fifo                      Set FIFO mode for EGL stream\n");
    LOG_MSG("-producer [n]              Set %d(video producer),\n", VIDEO_PRODUCER);
    LOG_MSG("                               %d(image producer),\n", IMAGE_PRODUCER);
    LOG_MSG("                               %d(gl    producer),\n", GL_PRODUCER);
#ifdef CUDA_SUPPORT
    LOG_MSG("                               %d(cuda  producer),\n", CUDA_PRODUCER);
#endif
    LOG_MSG("                           Default: %d\n", DEFAULT_PRODUCER);
    LOG_MSG("-consumer [n]              Set %d(video consumer),\n", VIDEO_CONSUMER);
    LOG_MSG("                               %d(image consumer),\n", IMAGE_CONSUMER);
    LOG_MSG("                               %d(gl    consumer),\n", GL_CONSUMER);
#ifdef CUDA_SUPPORT
    LOG_MSG("                               %d(cuda  consumer),\n", CUDA_CONSUMER);
#endif
#ifdef NVMEDIA_QNX
    LOG_MSG("                               %d(screen window consumer),\n", SCREEN_WINDOW_CONSUMER);
#endif
#ifdef EGLOUTPUT_SUPPORT
    LOG_MSG("                               %d(egloutput consumer),\n", EGLOUTPUT_CONSUMER);
#endif
    LOG_MSG("                           Default: %d\n", DEFAULT_CONSUMER);
    LOG_MSG("-metadata                  Enable metadata for EGL stream\n");
    LOG_MSG("-defaultdisplay            Use default display instead of window system\n");
    LOG_MSG("-enc [file]                Output 264 bitstream, may be set when consumer=%d %d\n", VIDEO_CONSUMER, IMAGE_CONSUMER);
    LOG_MSG("-standalone [n]            Set 0(not standalone, producer/consumer),\n");
    LOG_MSG("                               1(producer), 2(consumer),\n");
    LOG_MSG("                               3(cross partition producer), 4(cross partition consumer) default=0\n");
    LOG_MSG("-consumervm [n]            Set consumer VM ID for cross-partition producer\n");
    LOG_MSG("-w [window]                Display hardware window Id [0-2] (default=1)\n");
    LOG_MSG("-d [id]                    Display ID\n");
    LOG_MSG("-ot [type]                 Surface type: yuv420/yuv420p/rgba/yuv422p\n");
    LOG_MSG("                           yuv420 posts YUV420 semiplanar surfaces\n");
    LOG_MSG("                           use yuv420p for posting YUV 420 planar surfaces. ");
    LOG_MSG("Supported by cuda and image producers and consumers\n");
    LOG_MSG("                           use rgba for posting RGBA surfaces\n");
    LOG_MSG("                           use yuv422p for posting YUV 422 planar surfaces. ");
    LOG_MSG("Supported by cuda and image consumers in standalone consumer mode\n");
    LOG_MSG("                           Default: rgba\n");
    LOG_MSG("-shader [type]             shader type: yuv/rgb(default)\n");
    LOG_MSG("                           shader type can only be used when gl consumer enabled\n");
    LOG_MSG("-socketport                set port for cross process/partition eglstream communication[1024-49151]. Default: 8888\n");
    LOG_MSG("-ip                        set ip of consumer partition for cross partition eglstream communication\n");

    rt = GetAvailableDisplayDevices(&outputDevicesNum, &videoOutputs[0]);
    if(rt != NVMEDIA_STATUS_OK) {
        LOG_ERR("PrintUsage: Failed retrieving available video output devices\n");
        return;
    }

    LOG_MSG("\nAvailable display devices (%d):\n", outputDevicesNum);
    for(i = 0; i < outputDevicesNum; i++) {
        LOG_MSG("Display ID: %d \n", videoOutputs[i].displayId);
    }
}

int MainParseArgs(int argc, char **argv, TestArgs *args)
{
    int bLastArg = 0;
    int bDataAvailable = 0;
    int i;
    NvMediaBool displayDeviceEnabled;
    NvMediaStatus rt;
    NvMediaBool useShader = NV_FALSE;
    NvMediaBool validConsumerVm = NV_FALSE;
    NvMediaBool isColorformatYuv422 = NV_FALSE;
    NvU32 consumerType = CONSUMER_COUNT;

    NVM_SURF_FMT_DEFINE_ATTR(prodSurfFormatAttrs);
    NVM_SURF_FMT_DEFINE_ATTR(consSurfFormatAttrs);

    args->producerType = PRODUCER_COUNT;
    args->socketport = 8888;
    strncpy(args->ip, "127.0.0.1", 16);

    for (i = 1; i < argc; i++) {
        // check if this is the last argument
        bLastArg = ((argc - i) == 1);

        // check if there is data available to be parsed following the option
        bDataAvailable = (!bLastArg) && !(argv[i+1][0] == '-');

        if (argv[i][0] == '-') {
            if (strcmp(&argv[i][1], "h") == 0) {
                PrintUsage();
                return 0;
            }
            else if (strcmp(&argv[i][1], "fifo") == 0) {
                args->fifoMode = NV_TRUE;
            }
            else if (strcmp(&argv[i][1], "metadata") == 0) {
                args->metadata = NV_TRUE;
            }
            else if (strcmp(&argv[i][1], "v") == 0) {
                int logLevel = LEVEL_DBG;
                if(bDataAvailable) {
                    logLevel = atoi(argv[++i]);
                    if(logLevel < LEVEL_ERR || logLevel > LEVEL_DBG) {
                        LOG_INFO("MainParseArgs: Invalid logging level chosen (%d). ", logLevel);
                        LOG_INFO("           Setting logging level to LEVEL_ERR (0)\n");
                        logLevel = LEVEL_ERR;
                    }
                }
                SetLogLevel(logLevel);
            }
            else if (strcmp(&argv[i][1], "d") == 0) {
                if (bDataAvailable) {
                    if((sscanf(argv[++i], "%u", &args->displayId) != 1)) {
                        LOG_ERR("Err: Bad display id: %s\n", argv[i]);
                        return 0;
                    }
                    rt = CheckDisplayDeviceID(args->displayId, &displayDeviceEnabled);
                    if (rt != NVMEDIA_STATUS_OK) {
                        LOG_ERR("Err: Chosen display (%d) not available\n", args->displayId);
                        return 0;
                    }
                    args->displayEnabled = NV_TRUE;
                    LOG_DBG("ParseArgs: Chosen display: (%d) device enabled? %d \n",
                            args->displayId, displayDeviceEnabled);
                } else {
                    LOG_ERR("Err: -d must be followed by display id\n");
                    return 0;
                }
            }
            else if (strcmp(&argv[i][1], "consumer") == 0) {
                if (bDataAvailable) {
                    if (sscanf(argv[++i], "%u", &consumerType)) {
                        if (consumerType >= CONSUMER_COUNT){
                            LOG_ERR("ERR: -consumer must be set to 0-%d\n", CONSUMER_COUNT-1);
                            return 0;
                        }
                    } else {
                        LOG_ERR("ERR: -consumer must be set to 0-%d\n", CONSUMER_COUNT-1);
                    }
                } else {
                    LOG_ERR("ERR: -consumer must be set to 0-%d\n", CONSUMER_COUNT-1);
                    return 0;
                }
            }
            else if (strcmp(&argv[i][1], "producer") == 0) {
                if (bDataAvailable) {
                    if (sscanf(argv[++i], "%u", &(args->producerType))) {
                        if (args->producerType >= PRODUCER_COUNT){
                            LOG_ERR("ERR: -producer must be set to 0-%d\n", PRODUCER_COUNT-1);
                            return 0;
                        }
                    } else {
                        LOG_ERR("ERR: -producer must be set to 0-%d\n", PRODUCER_COUNT-1);
                    }
                } else {
                    LOG_ERR("ERR: -producer must be set to 0-%d\n", PRODUCER_COUNT-1);
                    return 0;
                }
            }
            else if (strcmp(&argv[i][1], "defaultdisplay") == 0)
            {
                if (!args->glConsumer && !args->egloutputConsumer && !args->screenConsumer &&
                    !args->glProducer) {
                    args->defaultDisplay = NV_TRUE;
                } else {
                    LOG_ERR("ERR: -defaultdisplay not allowed with GL/EglOutput/Screen consumer and GL producer");
                    return 0;
                }
            }
            else if (strcmp(&argv[i][1], "consumervm") == 0) {
                if(!bDataAvailable || !sscanf(argv[++i], "%u", &args->consumerVmId)) {
                    LOG_ERR("ERR: -consumervm must be set to proper value\n");
                    return 0;
                } else {
                    validConsumerVm = NV_TRUE;
                }
            }
            else if (strcmp(&argv[i][1], "enc") == 0 ) {
                LOG_DBG("Encode enabled\n");
                args->nvmediaEncoder = NV_TRUE;
                args->outfile = argv[++i];
            }
            else if (strcmp(&argv[i][1], "standalone") == 0 ) {
#ifndef NVMEDIA_GHSI
                if(!bDataAvailable || !sscanf(argv[++i], "%u", &args->standalone) || (args->standalone > 4)) {
                    LOG_ERR("ERR: -standalone must be followed by mode [0-4].\n");
#else
                if(!bDataAvailable || !sscanf(argv[++i], "%u", &args->standalone) || (args->standalone > 2)) {
                    LOG_ERR("ERR: -standalone must be followed by mode [0-2].\n");
#endif
                    return 0;
                }
            }
            else if (strcmp(&argv[i][1], "w") == 0) {
                if (!bDataAvailable || !sscanf(argv[++i], "%u", &args->windowId) || (args->windowId > 2)) {
                    LOG_ERR("ERR: -w must be followed by window id [0-2].\n");
                    return 0;
                }
            }
            else if (strcmp(&argv[i][1], "ip") == 0) {
                if (!bDataAvailable || !strncpy(args->ip, argv[++i], 16)) {
                    LOG_ERR("ERR: -ip must be followed by a valid IP like 12.0.0.1\n");
                    return 0;
                }
            }
            else if (strcmp(&argv[i][1], "socketport") == 0) {
                if (!bDataAvailable || !sscanf(argv[++i], "%u", &args->socketport)) {
                    LOG_ERR("ERR: -socketport must be followed by port number\n");
                    return 0;
                }
                if ((args->socketport < 1024) || (args->socketport > 49151)) {
                    LOG_ERR("ERR: Invalid socket port\n");
                    return 0;
                }
            }
        }
    }

    if (args->standalone == STANDALONE_CP_PRODUCER) {
        if(!strcmp(args->ip, "127.0.0.1")) {
            LOG_ERR("ERR: -ip must be specified with a valid IP like 12.0.0.1 for cross-partition producer\n");
            return 0;
        }
        if(validConsumerVm == NV_FALSE) {
            LOG_ERR("ERR: -consumervm must be specified for cross-partition producer\n");
            return 0;
        }
        if((args->glProducer == NV_TRUE) || (args->cudaProducer == NV_TRUE)) {
            LOG_ERR("ERR: Testing of GL and CUDA cross-partition producer is not supported in this app\n");
            return 0;
        }
    }

    if ((args->standalone == STANDALONE_PRODUCER) || (args->standalone == STANDALONE_CP_PRODUCER)) {
        if (consumerType >= CONSUMER_COUNT){
            LOG_ERR("ERR: -consumer must be specified for cross-process/cross-partition producer\n", CONSUMER_COUNT-1);
            return 0;
        }
        args->crossConsumerType = consumerType;
    } else {
        args->nvmediaConsumer      = (consumerType==VIDEO_CONSUMER ||
                                      consumerType==IMAGE_CONSUMER);
        args->nvmediaVideoConsumer = (consumerType==VIDEO_CONSUMER);
        args->nvmediaImageConsumer = (consumerType==IMAGE_CONSUMER);
        args->glConsumer           = (consumerType==GL_CONSUMER);
#ifdef CUDA_SUPPORT
        args->cudaConsumer         = (consumerType==CUDA_CONSUMER);
#endif
#ifdef NVMEDIA_QNX
        args->screenConsumer       = (consumerType==SCREEN_WINDOW_CONSUMER);
#endif
#ifdef EGLOUTPUT_SUPPORT
        args->egloutputConsumer    = (consumerType==EGLOUTPUT_CONSUMER);
#endif
    }

    if ((args->standalone == STANDALONE_CONSUMER) || (args->standalone == STANDALONE_CP_CONSUMER)) {
        if (args->producerType >= PRODUCER_COUNT){
            LOG_ERR("ERR: -producer must be specified for cross-process/cross-partition consumer\n", PRODUCER_COUNT-1);
            return 0;
        }
    } else {
        args->nvmediaVideoProducer = (args->producerType==VIDEO_PRODUCER);
        args->nvmediaImageProducer = (args->producerType==IMAGE_PRODUCER);
        args->nvmediaProducer =      (args->producerType==VIDEO_PRODUCER ||
                                      args->producerType==IMAGE_PRODUCER);
#ifdef CUDA_SUPPORT
        args->cudaProducer =         (args->producerType==CUDA_PRODUCER);
#endif
        args->glProducer =           (args->producerType==GL_PRODUCER);
    }

    if (args->nvmediaConsumer) {
        if ((args->displayEnabled != NV_TRUE) && (args->nvmediaEncoder != NV_TRUE)) {
            LOG_ERR("ERR: Either provide display ID to render or encode output file name\n");
            return 0;
        } else if ((args->displayEnabled == NV_TRUE) && (args->nvmediaEncoder == NV_TRUE)) {
            LOG_ERR("Both display and encode cannot be enabled simultaneously. Enabling encode option.\n");
            args->displayEnabled = NV_FALSE;
        }
    }

    args->consIsRGBA = NV_TRUE;
    args->prodIsRGBA = NV_TRUE;
    NVM_SURF_FMT_SET_ATTR_RGBA(prodSurfFormatAttrs,RGBA,UINT,8,PL);
    NVM_SURF_FMT_SET_ATTR_RGBA(consSurfFormatAttrs,RGBA,UINT,8,PL);
    args->shaderType = 0; // default, using RGB shader
    args->bSemiplanar = NV_TRUE;

    // The rest
    for(i = 0; i < argc; i++) {
        bLastArg = ((argc - i) == 1);

        // check if there is data available to be parsed following the option
        bDataAvailable = (!bLastArg) && !(argv[i+1][0] == '-');

        if (argv[i][0] == '-') {
            if (strcmp(&argv[i][1], "ot") == 0 ) {
                if(bDataAvailable) {
                    ++i;
                    if(!strcasecmp(argv[i], "yuv420")) {
                        args->consIsRGBA = NV_FALSE;
                        args->prodIsRGBA = NV_FALSE;
                        NVM_SURF_FMT_SET_ATTR_YUV(prodSurfFormatAttrs,YUV,420,SEMI_PLANAR,UINT,8,PL);
                        NVM_SURF_FMT_SET_ATTR_YUV(consSurfFormatAttrs,YUV,420,SEMI_PLANAR,UINT,8,PL);
                    } else if(!strcasecmp(argv[i], "rgba")) {
                        args->consIsRGBA = NV_TRUE;
                        args->prodIsRGBA = NV_TRUE;
                        NVM_SURF_FMT_SET_ATTR_RGBA(prodSurfFormatAttrs,RGBA,UINT,8,PL);
                        NVM_SURF_FMT_SET_ATTR_RGBA(consSurfFormatAttrs,RGBA,UINT,8,PL);
                    } else if(!strcasecmp(argv[i], "yuv420p")) {
                        args->consIsRGBA = NV_FALSE;
                        args->prodIsRGBA = NV_FALSE;
                        args->bSemiplanar = NV_FALSE;
                        NVM_SURF_FMT_SET_ATTR_YUV(prodSurfFormatAttrs,YUV,420,PLANAR,UINT,8,PL);
                        NVM_SURF_FMT_SET_ATTR_YUV(consSurfFormatAttrs,YUV,420,PLANAR,UINT,8,PL);
                    } else if(!strcasecmp(argv[i], "yuv422p")) {
                        args->consIsRGBA = NV_FALSE;
                        args->prodIsRGBA = NV_FALSE;
                        args->bSemiplanar = NV_FALSE;
                        isColorformatYuv422 = NV_TRUE;
                        NVM_SURF_FMT_SET_ATTR_YUV(prodSurfFormatAttrs,YUV,422,PLANAR,UINT,8,PL);
                        NVM_SURF_FMT_SET_ATTR_YUV(consSurfFormatAttrs,YUV,422,PLANAR,UINT,8,PL);
                    } else {
                        printf("ERR: ParseArgs: Unknown output surface type encountered: %s\n", argv[i]);
                        return 0;
                    }
                } else {
                    printf("ERR: ParseArgs: -ot must be followed by surface type\n");
                    return 0;
                }
            } else if(strcmp(&argv[i][1], "f") == 0) {
                // Input file name
                if(bDataAvailable) {
                    args->infile = argv[++i];
                } else {
                    LOG_ERR("ParseArgs: -f must be followed by input file name\n");
                    return 0;
                }
            } else if(strcmp(&argv[i][1], "fr") == 0) {
                if(bDataAvailable) {
                    if((sscanf(argv[++i], "%ux%u", &args->inputWidth, &args->inputHeight) != 2)) {
                        LOG_ERR("ParseArgs: Bad output resolution: %s\n", argv[i]);
                        return 0;
                    }
                } else {
                    LOG_ERR("ParseArgs: -fr must be followed by resolution\n");
                    return 0;
                }
            } else if(strcmp(&argv[i][1], "pl") == 0) {
                args->pitchLinearOutput = NV_TRUE;
            } else if (strcmp(&argv[i][1], "l") == 0) {
                if (argv[i+1]) {
                    int loop;
                    if (sscanf(argv[++i], "%d", &loop) && loop >= -1 && loop != 0) {
                        args->prodLoop = loop;
                    } else {
                        printf("ERR: Invalid loop count encountered (%s)\n", argv[i]);
                    }
                } else {
                    printf("ERR: -l must be followed by loop count.\n");
                    return 0;
                }
            } else if (strcmp(&argv[i][1], "n") == 0) {
                if (bDataAvailable) {
                    int frameCount;
                    if (sscanf(argv[++i], "%d", &frameCount)) {
                        args->prodFrameCount = frameCount;
                    } else {
                        LOG_DBG("ERR: -n must be followed by decode frame count.\n");
                    }
                } else {
                    LOG_DBG("ERR: -n must be followed by frame count.\n");
                    return 0;
                }
            } else if(strcmp(&argv[i][1], "shader") == 0) {
                if(bDataAvailable) {
                    i++;
                    if(!strcasecmp(argv[i], "yuv")) {
                        args->shaderType = NV_TRUE;
                        LOG_DBG("using yuv shader.\n");
                    } else if(!strcasecmp(argv[i], "rgb")) {
                        args->shaderType = NV_FALSE;
                        LOG_DBG("using rgb shader.\n");
                    } else {
                        LOG_ERR("ParseArgs: -shader unknown shader type)\n");
                        return 0;
                    }
                    useShader = NV_TRUE;
                } else {
                    LOG_ERR("ParseArgs: -shader rgb | yuv\n");
                    return 0;
                }
            }
        }
    }

    if (!args->pitchLinearOutput){
        prodSurfFormatAttrs[NVM_SURF_ATTR_LAYOUT].value = NVM_SURF_ATTR_LAYOUT_BL;
        consSurfFormatAttrs[NVM_SURF_ATTR_LAYOUT].value = NVM_SURF_ATTR_LAYOUT_BL;
    }
    args->prodSurfaceType = NvMediaSurfaceFormatGetType(prodSurfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);
    args->consSurfaceType = NvMediaSurfaceFormatGetType(consSurfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);

    if ((args->nvmediaImageProducer || args->cudaProducer) &&
       !(args->inputWidth && args->inputHeight)) {
        LOG_ERR("Input File Resolution must be specified for image/CUDA producer (use -fr option)\n");
        return 0;
    }
    //check vadility, nvmediaEncoder can be used only when nvmedia consumer is enable
    if ( args->nvmediaEncoder && (!args->nvmediaConsumer)){
        LOG_ERR("Please use nvmedia consumer when doing encoding\n");
        return 0;
    }

    if (args->cudaConsumer && (args->standalone!=2) && (args->standalone!=4) &&
       !(args->nvmediaVideoProducer || args->nvmediaImageProducer || args->cudaProducer)) {
        LOG_ERR("Invalid EGL pipeline, please use video/image or CUDA producer with CUDA consumer\n");
        return 0;
    }

    if (args->metadata) {
        if ((args->standalone == 0) && !(args->nvmediaImageProducer && args->nvmediaImageConsumer)) {
            LOG_ERR("Please enable metadata only with image producer and image consumer\n");
            return 0;
        } else if (((args->standalone == 1) || (args->standalone == 3)) && (!args->nvmediaImageProducer)) {
            LOG_ERR("Please enable metadata only with image producer and image consumer\n");
            return 0;
        } else if (((args->standalone == 2) || (args->standalone == 4)) && (!args->nvmediaImageConsumer)) {
            LOG_ERR("Please enable metadata only with image producer and image consumer\n");
            return 0;
        }
    }

    if(!args->glConsumer && useShader) {
        LOG_INFO("-shader command only use with gl consumer enabled, ignore shader command here\n");
    }

    if(args->glConsumer && args->shaderType &&
            (consSurfFormatAttrs[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA)) {
        LOG_INFO("rgba surface type can only use rgb shader, set shader to rgb\n");
        args->shaderType = NV_FALSE;
    }

    if ((args->standalone != STANDALONE_CONSUMER) && (args->standalone != STANDALONE_CP_CONSUMER)) {
        if ((args->bSemiplanar == NV_FALSE) &&
        !((args->cudaProducer) || (args->nvmediaImageProducer))) {
            LOG_ERR("Invalid surface type, YUV planar is supported only with CUDA or image producer\n");
            return 0;
        }
    }

    if ((args->standalone != STANDALONE_CONSUMER) && (args->standalone != STANDALONE_CP_CONSUMER)) {
        if (NV_TRUE == isColorformatYuv422) {
            LOG_ERR("Invalid surface type, YUV422 planar is only supported"\
                    "in standalone consumer\n");
            return 0;
        }
    }

    if ((args->bSemiplanar == NV_FALSE) && (args->nvmediaVideoConsumer)) {
        LOG_ERR("YUV planar is not supported for Video Consumer");
        return 0;
    }
    return 1;
}
