/*
 * nvmimage_consumer.c
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
// DESCRIPTION:   Simple image consumer rendering sample app
//

#include "nvm_consumer.h"
#include "log_utils.h"

#define IMAGE_BUFFERS_POOL_SIZE      4
#define BUFFER_POOL_TIMEOUT          100

#define DISPLAY_ENABLED 0

#if defined(EXTENSION_LIST)
EXTENSION_LIST(EXTLST_EXTERN)
#endif

#if 0
static NvU32
procThreadFunc (
    void *data)
{
    test_nvmedia_consumer_display_s *display = (test_nvmedia_consumer_display_s *)data;
    NvMediaTime timeStamp;
    NvMediaImage *image = NULL;
    int i = 0;
	//NvU64 td;

    if(!display) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return 0;
    }

    LOG_DBG("NVMedia image consumer thread is active\n");

    while(!display->quit) {
        for (i=0; i<4; i++) {
            EGLint streamState = 0;
            if(!eglQueryStreamKHR(
                        display->eglDisplay,
                        display->eglStream[i],
                        EGL_STREAM_STATE_KHR,
                        &streamState)) {
                LOG_ERR("Nvmedia image consumer, eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
            }

            if(streamState == EGL_STREAM_STATE_DISCONNECTED_KHR) {
                LOG_ERR("Nvmedia image Consumer: - EGL_STREAM_STATE_DISCONNECTED_KHR received\n");
                display->quit = NV_TRUE;
                goto done;
            }
            if(streamState != EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR) {
                usleep(1000);
                continue;
            }

            //usleep(200000);

            if(NVMEDIA_STATUS_OK !=
                    NvMediaEglStreamConsumerAcquireImage(display->consumer[i], &image, NVMEDIA_EGL_STREAM_TIMEOUT_INFINITE, &timeStamp)) {
                LOG_ERR("Nvmedia image Consumer: - image acquire failed\n");
                display->quit = NV_TRUE;
                goto done;
            }
            //GetTimeMicroSec(&td);
            //LOG_ERR("%u\n", td);

            if (image) {
                LOG_ERR("%s %d\n", __func__, __LINE__);
                NvMediaEglStreamConsumerReleaseImage(display->consumer[i], image);
            }
        }
    }
done:
    display->procThreadExited = NV_TRUE;
    *display->consumerDone = NV_TRUE;
    return 0;
}
#endif

static test_nvmedia_consumer_display_s* g_display;

int getImgBuffer(unsigned char** buf1, unsigned char** buf2, unsigned char** buf3, unsigned char** buf4)
{
    NvMediaImage *image = NULL;
    NvMediaTime timeStamp;
    EGLint streamState = 0;
    test_nvmedia_consumer_display_s *display = g_display;
	NvMediaImageSurfaceMap surfaceMap;
	ImageBuffer *outputImageBuffer = NULL;
    unsigned char** inputBuffer[] = {buf1, buf2, buf3, buf4};
    int i = 0;
	NvU64 td, te;

	//LOG_ERR("get image buffer\n");

	if(display->quit) {
		LOG_ERR("%s %d %d\n", __func__, __LINE__, display->quit);
		goto fail;
	}
    //GetTimeMicroSec(&td);

    for (i=0; i<4; ++i) {
        if(!eglQueryStreamKHR(
                    display->eglDisplay,
                    display->eglStream[i],
                    EGL_STREAM_STATE_KHR,
                    &streamState)) {
            LOG_ERR("Nvmedia image consumer, eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
            goto fail;
        }

        if(streamState == EGL_STREAM_STATE_DISCONNECTED_KHR) {
            LOG_ERR("Nvmedia image Consumer: - EGL_STREAM_STATE_DISCONNECTED_KHR received\n");
            display->quit = NV_TRUE;
            goto fail;
        }

        if(NVMEDIA_STATUS_OK !=
                NvMediaEglStreamConsumerAcquireImage(display->consumer[i], &image, NVMEDIA_EGL_STREAM_TIMEOUT_INFINITE, &timeStamp)) {
            LOG_ERR("Nvmedia image Consumer: - image acquire failed\n");
            //display->quit = NV_TRUE;
            //goto fail;
        }

        if (image) {
            //LOG_ERR("%s %d\n", __func__, __LINE__);
            //GetTimeMicroSec(&td);
            //LOG_ERR("%u\n", td);
            NvMediaStatus status;
            char *env;

            if ((status = NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_READ, &surfaceMap)) !=
                    NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: NvMediaImageLock failed %d\n", __func__, status);
                goto fail;
            }
            //LOG_ERR("%s %d\n", __func__, __LINE__);
            //NvU32 srcPitch = 1920*4;
            NvU32 srcPitch = 960*4;
            if(NVMEDIA_STATUS_OK != (status = NvMediaImageGetBits(image, NULL, (void **)inputBuffer[i], &srcPitch))) {
                LOG_ERR("Nvmedia image get bits fail %d\n", status);
                goto fail;
            }
            //LOG_ERR("%s %d\n", __func__, __LINE__);
            NvMediaImageUnlock(image);
		    //LOG_ERR("%s %d %d %p\n", __func__, __LINE__, i, image);

            NvMediaEglStreamConsumerReleaseImage(display->consumer[i], image);
        }
    }
    //GetTimeMicroSec(&te);
    //LOG_ERR("%d\n", te-td);

	LOG_DBG("get one image\n");

	return 0;

fail:
	if(outputImageBuffer)
		BufferPool_ReleaseBuffer(display->outputBuffersPool, outputImageBuffer);
	return -1;
}

//Image display init
int image_display_init(volatile NvBool *consumerDone,
        test_nvmedia_consumer_display_s *display,
        EGLDisplay eglDisplay, EGLStreamKHR eglStream,
        NvU32 eglNum
        )
{
    display->consumerDone = consumerDone;
    display->metadataEnable = NV_FALSE;
    display->eglDisplay = eglDisplay;
    display->eglStream[eglNum] = eglStream;
    display->encodeEnable = NV_FALSE;
	display->outputBuffersPool = NULL;
    if (eglNum == 0) {
        display->device = NvMediaDeviceCreate();
        if (!display->device) {
            LOG_DBG("image_display_init: Unable to create device\n");
            return NV_FALSE;
        } 
        display->blitter = NvMedia2DCreate(display->device);
        if(!display->blitter) {
            LOG_DBG("%s: Failed to create NvMedia 2D blitter\n", __func__);
            return NV_FALSE;;
        }
    }
    display->consumer[eglNum] = NvMediaEglStreamConsumerCreate(
        display->device,
        display->eglDisplay,
        display->eglStream[eglNum],
        0x8a);
    if(!display->consumer) {
        LOG_ERR("image_display_init: Unable to create consumer\n");
        return NV_FALSE;
    }

#if 0
    if (IsFailed(NvThreadCreate(&display->procThread, &procThreadFunc, (void *)display, NV_THREAD_PRIORITY_NORMAL))) {
        LOG_ERR("Nvmedia image consumer init: Unable to create process thread\n");
        display->procThreadExited = NV_TRUE;
        return NV_FALSE;
    }
#endif
	display->quit = NV_FALSE;
    if (eglNum == 3) {
        g_display = (test_nvmedia_consumer_display_s*)malloc(sizeof(test_nvmedia_consumer_display_s));
        memcpy(g_display, display, sizeof(test_nvmedia_consumer_display_s));
        LOG_ERR("%s %d 0x%p\n", __func__, __LINE__, g_display->consumer[eglNum]);
    }
    return NV_TRUE;
}

void image_display_Deinit(test_nvmedia_consumer_display_s *display)
{
    NvU32 i;
    LOG_DBG("image_display_Deinit: start\n");
    display->quit = NV_TRUE;

    //Close IDP
    if(display && display->imageDisplay) {
        NvMediaIDPDestroy(display->imageDisplay);
        display->imageDisplay = NULL;
    }

   if(display->blitter)
        NvMedia2DDestroy(display->blitter);

    if(display->blitParams) {
        free(display->blitParams);
        display->blitParams = NULL;
    }

    if(display->outputBuffersPool)
        BufferPool_Destroy(display->outputBuffersPool);

    if(display->consumer) {
        NvMediaEglStreamConsumerDestroy(display->consumer);
        for (i=0; i<4; i++) {
            display->consumer[i] = NULL;
        }
    }

    LOG_DBG("image_display_Deinit: consumer destroy\n");
    if(display->device) {
        NvMediaDeviceDestroy(display->device);
        display->device = NULL;
    }
    LOG_DBG("image_display_Deinit: done\n");
}

void image_display_Stop(test_nvmedia_consumer_display_s *display) {
    display->quit = 1;
    if (display->procThread) {
        LOG_DBG("wait for nvmedia image consumer thread exit\n");
        NvThreadDestroy(display->procThread);
    }
}

void image_display_Flush(test_nvmedia_consumer_display_s *display) {
    NvMediaImage *image = NULL;
    NvMediaTime timeStamp;
    NvMediaImage *releaseFrames[IMAGE_BUFFERS_POOL_SIZE] = {0};
    NvMediaImage **releaseList = &releaseFrames[0];
    EGLint streamState = 0;

    do {
        if(!eglQueryStreamKHR(
                display->eglDisplay,
                display->eglStream,
                EGL_STREAM_STATE_KHR,
                &streamState)) {
            LOG_ERR("Nvmedia video consumer, eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
        }

        if(streamState == EGL_STREAM_STATE_DISCONNECTED_KHR) {
            break;
        }

        NvMediaEglStreamConsumerAcquireImage(display->consumer,
                                             &image,
                                             16,
                                             &timeStamp);
        if(image) {
            LOG_DBG("%s: EGL Consumer: Release image %p (display->consumer)\n", __func__, image);
            NvMediaEglStreamConsumerReleaseImage(display->consumer, image);
        }
    } while(image);

    releaseList = &releaseFrames[0];
    NvMediaIDPFlip(display->imageDisplay,   // display
                   NULL,                    // image
                   NULL,                    // source rectangle
                   NULL,                    // destination rectangle,
                   releaseList,             // release list
                   NULL);                   // time stamp
    while (*releaseList) {
        ImageBuffer *buffer;
        buffer = (ImageBuffer*)(*releaseList)->tag;
        BufferPool_ReleaseBuffer(buffer->bufferPool, buffer);
        *releaseList = NULL;
        releaseList++;
    }
}
