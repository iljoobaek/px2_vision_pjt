/*
 * testmain.c
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
// DESCRIPTION:   Simple EGL stream sample app
//
//
#include <eglstrm_setup.h>
#include <cmdline.h>
#include "nvmedia_core.h"
#include "nvmedia_surface.h"
#include "egl_utils.h"
#include "nvmimg_producer.h"
#include "nvmvid_producer.h"
#include "gl_producer.h"
#include "gl_consumer.h"
#ifdef NVMEDIA_QNX
#include "screen_consumer.h"
#endif
#include "nvm_consumer.h"
#ifdef EGLOUTPUT_SUPPORT
#include "egloutput_consumer.h"
#endif
#ifdef CUDA_SUPPORT
#include "cuda_consumer.h"
#include "cuda_producer.h"
#endif

#if defined(EXTENSION_LIST)
EXTENSION_LIST(EXTLST_EXTERN)
#endif

NvBool signal_stop = 0;

#define SET_VERSION(x, _major, _minor) \
    x.major = _major; \
    x.minor = _minor;

static void
sig_handler(int sig)
{
    signal_stop = NV_TRUE;
    printf("Signal: %d\n", sig);
}

static NvMediaStatus
CheckVersion(void)
{
    NvMediaVersion version;
    NvMediaStatus status = NVMEDIA_STATUS_OK;

    memset(&version, 0, sizeof(NvMediaVersion));
    status = NvMediaCoreGetVersion(&version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if((version.major != NVMEDIA_CORE_VERSION_MAJOR) ||
       (version.minor != NVMEDIA_CORE_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible core version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_CORE_VERSION_MAJOR, NVMEDIA_CORE_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            version.major, version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&version, 0, sizeof(NvMediaVersion));
    status = NvMediaImageGetVersion(&version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if((version.major != NVMEDIA_IMAGE_VERSION_MAJOR) ||
       (version.minor != NVMEDIA_IMAGE_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible image version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_IMAGE_VERSION_MAJOR, NVMEDIA_IMAGE_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            version.major, version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&version, 0, sizeof(NvMediaVersion));
    status = NvMediaEglStreamGetVersion(&version);
    if(status != NVMEDIA_STATUS_OK)
        return status;

    if((version.major != NVMEDIA_EGLSTREAM_VERSION_MAJOR) ||
       (version.minor != NVMEDIA_EGLSTREAM_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible EGLStream version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_EGLSTREAM_VERSION_MAJOR, NVMEDIA_EGLSTREAM_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            version.major, version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&version, 0, sizeof(NvMediaVersion));
    status = NvMedia2DGetVersion(&version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if ((version.major != NVMEDIA_2D_VERSION_MAJOR) ||
        (version.minor != NVMEDIA_2D_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible 2D version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_2D_VERSION_MAJOR, NVMEDIA_2D_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            version.major, version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&version, 0, sizeof(NvMediaVersion));
    status = NvMediaIDPGetVersion(&version);
    if (status != NVMEDIA_STATUS_OK)
        return status;
    if (version.major != NVMEDIA_IDP_VERSION_MAJOR ||
        version.minor != NVMEDIA_IDP_VERSION_MINOR) {
        LOG_ERR("%s: Incompatible IDP version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
                NVMEDIA_IDP_VERSION_MAJOR, NVMEDIA_IDP_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
                version.major, version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    return status;
}

int main(int argc, char **argv)
{
    int         failure = 1;
    EglUtilState *eglUtil = NULL;
    EGLStreamKHR eglStream=EGL_NO_STREAM_KHR;
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    TestArgs args;
    volatile NvBool producerDone = 0;
    volatile NvBool consumerDone = 0;

    char *opMode[] = { "Producer/Consumer", "Producer", "Consumer", "Cross-partition Producer", "Cross-partition Consumer" };

    GLConsumer glConsumer;
#ifdef NVMEDIA_QNX
    ScreenConsumer screenConsumer;
#endif
    test_nvmedia_consumer_display_s video_display;
#ifdef CUDA_SUPPORT
    test_cuda_consumer_s cudaConsumerTest;
#endif

    memset(&args, 0, sizeof(TestArgs));
    memset(&video_display, 0, sizeof(test_nvmedia_consumer_display_s));
    args.windowId = 1;  //default windowId

    // Hook up Ctrl-C handler
    signal(SIGINT, sig_handler);

    if (!MainParseArgs(argc, argv, &args)) {
        return failure;
    }

    if(CheckVersion() != NVMEDIA_STATUS_OK) {
        return failure;
    }

    printf("Operation mode: %s\n", opMode[args.standalone]);

    EglUtilOptions options;
    memset(&options, 0, sizeof(EglUtilOptions));
    options.displayId = args.displayId;
    options.vidConsumer = args.nvmediaVideoConsumer;

    if (args.defaultDisplay != NV_TRUE) {
        eglUtil = EGLUtilInit(&options);
        if (!eglUtil) {
            LOG_ERR("main - failed to initialize egl \n");
            goto done;
        }
        eglDisplay = eglUtil->display;
    } else {
        eglDisplay = EGLUtilDefaultDisplayInit();
        if (EGL_NO_DISPLAY == eglDisplay) {
            LOG_ERR("main - failed to initialize egl \n");
            goto done;
        }
    }

    eglStream = EGLStreamInit(eglDisplay, &args);
    if(!eglStream) {
        goto done;
    }

    //Init consumer
    if (args.glConsumer) {
        glConsumer_init(&consumerDone, &glConsumer, eglUtil->display, eglStream, eglUtil, &args);
    } else if (args.screenConsumer) {
#ifdef NVMEDIA_QNX
        screenConsumer_init(&consumerDone, &screenConsumer, eglUtil->display, eglStream, &args);
#else
        LOG_ERR("main - screen consumer not available\n");
#endif
    } else if (args.nvmediaVideoConsumer) {
        LOG_DBG("main - video_display_init\n");
        if(!video_display_init(&consumerDone, &video_display, eglDisplay, eglStream, &args)) {
            LOG_ERR("main: Display init failed\n");
            goto done;
        }
    } else if (args.nvmediaImageConsumer) {
        LOG_DBG("main - image_display_init\n");
        if(!image_display_init(&consumerDone, &video_display, eglDisplay, eglStream, &args)) {
            LOG_ERR("main: image Display init failed\n");
            goto done;
        }
    } else if (args.cudaConsumer == NV_TRUE) {
#ifdef CUDA_SUPPORT
        if (!cuda_consumer_init(&consumerDone, &cudaConsumerTest, eglDisplay, eglStream, &args)) {
            LOG_ERR("main: cuda conusmer init failed\n");
            goto done;
        }
#else
        goto done;
#endif
    } else if (args.egloutputConsumer) {
#if EGLOUTPUT_SUPPORT
        if (egloutputConsumer_init(eglUtil->display, eglStream) != NVMEDIA_STATUS_OK) {
            LOG_ERR("main: egloutput consumer init failed\n");
            goto done;
        }
#else
        LOG_ERR("main - egloutput consumer not available\n");
        goto done;
#endif
    }

    // Initialize producer
#ifndef NVMEDIA_GHSI
    if((args.standalone != STANDALONE_CONSUMER)  && (args.standalone != STANDALONE_CP_CONSUMER)){
#else
    if(args.standalone != STANDALONE_CONSUMER){
#endif
        EGLint streamState = 0;
        while(!signal_stop && streamState != EGL_STREAM_STATE_CONNECTING_KHR) {
           if(!eglQueryStreamKHR(
               eglDisplay,
               eglStream,
               EGL_STREAM_STATE_KHR,
               &streamState)) {
               LOG_ERR("main: before init producer, eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
            }
        }
        if(args.glProducer){
            LOG_DBG("main - GearProducerInit - Before\n");
            if (!GearProducerInit(&producerDone, eglUtil->display, eglStream, &args)) {
                LOG_ERR("main: GearProducerInit failed\n");
                goto consumer_cleanup;
            }
            LOG_DBG("main - GearProducerInit - After\n");
        } else if(args.nvmediaVideoProducer){
            LOG_DBG("main - VideoDecoderInit - Before\n");
            if(!VideoDecoderInit(&producerDone,
                                 eglDisplay,
                                 eglStream, &args)) {
                LOG_ERR("main: VideoDecoderInit failed\n");
                goto consumer_cleanup;
            }
            LOG_DBG("main - VideoDecoderInit - After\n");
        } else if(args.nvmediaImageProducer) {
            LOG_DBG("main - Image2DInit - Before\n");
            if(!Image2DInit(&producerDone, eglDisplay, eglStream, &args)) {
                LOG_ERR("main: Image2DInit failed\n");
                goto consumer_cleanup;
            }
            LOG_DBG("main - Image2DInit - After\n");
        } else if (args.cudaProducer) {
#ifdef CUDA_SUPPORT
            LOG_DBG("main - CudaProducerInit - Before\n");
            if(!CudaProducerInit(&producerDone, eglDisplay, eglStream, &args)) {
                LOG_ERR("main: CudaProducerInit failed\n");
                goto consumer_cleanup;
            }
            LOG_DBG("main - CudaProducerInit - After\n");
#else
            goto consumer_cleanup;
#endif
        }
    }

    // wait for signal_stop or producer done
    while(!signal_stop && !producerDone && !consumerDone) {
        usleep(1000);
    }

    LOG_DBG("main - stop producer thread and consumer thread \n");
    if(args.glProducer) {
        GearProducerStop();
    } else if(args.nvmediaVideoProducer) {
        VideoDecoderStop();
    } else if(args.nvmediaImageProducer) {
        Image2DproducerStop();
    }

consumer_cleanup:
    if (args.nvmediaVideoConsumer)
        video_display_Stop(&video_display);
    else if (args.nvmediaImageConsumer)
        image_display_Stop(&video_display);
    else if (args.glConsumer)
        glConsumerStop(&glConsumer);
    else if (args.screenConsumer) {
#ifdef NVMEDIA_QNX
        screenConsumerStop(&screenConsumer);
#endif
    }
    LOG_DBG("main - stop produer thread and consumer thread: Done \n");

    LOG_DBG("main - flush producer and consumer eglstream \n");
    if(args.nvmediaVideoConsumer)
       video_display_Flush(&video_display);
    else if(args.nvmediaImageConsumer)
       image_display_Flush(&video_display);
    else if(args.glConsumer)
        glConsumerFlush(&glConsumer);

//! [docs_eglstream:destroy_producer]
    if(args.nvmediaVideoProducer)
       VideoDecoderFlush();
    else if(args.nvmediaImageProducer)
       Image2DproducerFlush();

    LOG_DBG("main - flush produer and consumer eglstream: Done\n");

    LOG_DBG("main - program end, clean up start\n");

    if(args.nvmediaVideoProducer)
       VideoDecoderDeinit();
    else if (args.nvmediaImageProducer)
       Image2DDeinit();
    else if (args.cudaProducer) {
#ifdef CUDA_SUPPORT
       CudaProducerDeinit();
#endif
    }
    else if(args.glProducer) {
        GearProducerDeinit();
    }
//! [docs_eglstream:destroy_producer]

    if (args.nvmediaVideoConsumer)
       video_display_Deinit(&video_display);
    else if (args.nvmediaImageConsumer) {
       image_display_Deinit(&video_display);
    } else if (args.cudaConsumer == NV_TRUE) {
#ifdef CUDA_SUPPORT
        cuda_consumer_Deinit(&cudaConsumerTest);
        LOG_DBG ("Total number of EGL stream Frame received: %d\n", cudaConsumerTest.frameCount);
#endif
    } else if(args.glConsumer) {
          glConsumerCleanup(&glConsumer);
    }
    failure = 0;
done:
    EGLStreamFini(eglDisplay, eglStream);
    if (args.defaultDisplay == NV_TRUE) {
        EGLUtilDefaultDisplayDeinit(eglDisplay);
    } else {
        if(eglUtil) {
            EGLUtilDeinit(eglUtil);
        }
    }
    LOG_DBG("main - clean up done\n");

    return failure;
}
