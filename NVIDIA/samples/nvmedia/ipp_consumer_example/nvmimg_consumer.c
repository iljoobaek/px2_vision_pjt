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
extern struct timespec tpstart;
extern struct timespec tpend;
extern float diff_sec, diff_nsec;
extern int num_f;
extern float sum_time, max_time;

#if 0
static NvU32
procThreadFunc (
    void *data)
{
    test_nvmedia_consumer_display_s *display = (test_nvmedia_consumer_display_s *)data;
    NvMediaTime timeStamp;
    NvMediaImage *image = NULL;
    NvMediaImage *releaseFrames[IMAGE_BUFFERS_POOL_SIZE] = {0};
    NvMediaImage **releaseList = &releaseFrames[0];
	//NvU64 td;

    if(!display) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return 0;
    }

    LOG_DBG("NVMedia image consumer thread is active\n");

    while(!display->quit) {
        EGLint streamState = 0;
        if(!eglQueryStreamKHR(
                display->eglDisplay,
                display->eglStream,
                EGL_STREAM_STATE_KHR,
                &streamState)) {
            LOG_ERR("Nvmedia image consumer, eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
        }

        if(streamState == EGL_STREAM_STATE_DISCONNECTED_KHR) {
            LOG_DBG("Nvmedia image Consumer: - EGL_STREAM_STATE_DISCONNECTED_KHR received\n");
            display->quit = NV_TRUE;
            goto done;
        }
        if(streamState != EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR) {
            usleep(1000);
            continue;
        }

		//usleep(200000);

        if(NVMEDIA_STATUS_OK !=
                NvMediaEglStreamConsumerAcquireImage(display->consumer, &image, 16, &timeStamp)) {
            LOG_DBG("Nvmedia image Consumer: - image acquire failed\n");
            display->quit = NV_TRUE;
            goto done;
        }
		//GetTimeMicroSec(&td);
		//LOG_ERR("%u\n", td);

		LOG_DBG("%s %d\n", __func__, __LINE__);
		if (image) {
			LOG_DBG("%s %d\n", __func__, __LINE__);
            NvMediaEglStreamConsumerReleaseImage(display->consumer, image);
        }
    }
done:
    display->procThreadExited = NV_TRUE;
    *display->consumerDone = NV_TRUE;
    return 0;
}
#endif

static test_nvmedia_consumer_display_s* g_display;

int getImgBuffer(unsigned char* inputBuffer)
{
    NvMediaImage *image = NULL;
    NvMediaTime timeStamp;
    EGLint streamState = 0;
    test_nvmedia_consumer_display_s *display = g_display;
	NvMediaImageSurfaceMap surfaceMap;
	ImageBuffer *outputImageBuffer = NULL;

	LOG_DBG("get image buffer\n");

	if(display->quit) {
		LOG_ERR("%s %d %d\n", __func__, __LINE__, display->quit);
		goto fail;
	}

	if(!eglQueryStreamKHR(
				display->eglDisplay,
				display->eglStream,
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
			NvMediaEglStreamConsumerAcquireImage(display->consumer, &image, NVMEDIA_EGL_STREAM_TIMEOUT_INFINITE, &timeStamp)) {
		LOG_ERR("Nvmedia image Consumer: - image acquire failed\n");
		display->quit = NV_TRUE;
		goto fail;
	}

	if (image) {
		LOG_DBG("%s %d\n", __func__, __LINE__);
		NvMediaStatus status;
		char *env;
#if 0
		//Create output buffer pool for image 2d blit for vertical flip
		if (display->outputBuffersPool == NULL) {
			ImageBufferPoolConfigNew imagesPoolConfig;
			NVM_SURF_FMT_DEFINE_ATTR(attr);
			memset(&imagesPoolConfig, 0, sizeof(ImageBufferPoolConfigNew));
			imagesPoolConfig.device = display->device;

			status = NvMediaSurfaceFormatGetAttrs(image->type,
					attr,
					NVM_SURF_FMT_ATTR_MAX);
			if (status != NVMEDIA_STATUS_OK) {
				LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
				return NV_FALSE;
			}

			NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);
			NVM_SURF_FMT_SET_ATTR_RGBA(surfFormatAttrs,RGBA,UINT,8,PL);
			imagesPoolConfig.surfType = NvMediaSurfaceFormatGetType(surfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);

			imagesPoolConfig.surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
			imagesPoolConfig.surfAllocAttrs[0].value = image->width;
			imagesPoolConfig.surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
			imagesPoolConfig.surfAllocAttrs[1].value = image->height;
			imagesPoolConfig.numSurfAllocAttrs = 2;

			imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].type = NVM_SURF_ATTR_CPU_ACCESS;
			imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].value = NVM_SURF_ATTR_CPU_ACCESS_CACHED;
			//imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].type = NVM_SURF_ATTR_ALLOC_TYPE;
			//imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
			imagesPoolConfig.numSurfAllocAttrs += 1;
			if ((env = getenv("DISPLAY_VM"))) {
				imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].type = NVM_SURF_ATTR_PEER_VM_ID;
				imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].value = atoi(env);
				imagesPoolConfig.numSurfAllocAttrs += 1;
			}


			if(IsFailed(status = BufferPool_Create_New(&display->outputBuffersPool,    // Buffer pool
							IMAGE_BUFFERS_POOL_SIZE, // Capacity
							BUFFER_POOL_TIMEOUT,     // Timeout
							IMAGE_BUFFER_POOL,       // Buffer pool type
							&imagesPoolConfig))) {
				LOG_ERR("%s: Create output buffer pool failed %d\n",__func__, status);
				goto fail;
			}
		}

		if(IsFailed(BufferPool_AcquireBuffer(display->outputBuffersPool,
						(void *)&outputImageBuffer))){
			LOG_ERR("%s: Output BufferPool_AcquireBuffer failed \n",__func__);
			goto fail;
		}

		if(IsFailed(NvMedia2DBlitEx(display->blitter,
						outputImageBuffer->image,
						NULL,
						image,
						NULL,
						NULL,
						NULL))) {
			LOG_ERR("%s:  output image NvMedia2DBlitEx failed \n",__func__);
			goto fail;
	2}

#endif
		if ((status = NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_READ, &surfaceMap)) !=
				NVMEDIA_STATUS_OK) {
			LOG_ERR("%s: NvMediaImageLock failed %d\n", __func__, status);
			goto fail;
		}
		//LOG_ERR("%s %d %d %d\n", __func__, __LINE__, surfaceMap.height, surfaceMap.width);
		NvU32 srcPitch = 3840*4;
		//NvU32 srcPitch = 960*4;

		/* ***** START TIMING ***** */
		//clock_gettime(CLOCK_MONOTONIC, &tpstart);
		//printf("%ld %ld\n", tpstart.tv_sec, tpstart.tv_nsec);
		/* ***** START TIMING ***** */
		//NvU64 tbegin = 0, tend = 0;
		//GetTimeMicroSec(&tbegin);

		if(NVMEDIA_STATUS_OK != (status = NvMediaImageGetBits(image, NULL, (void **)&inputBuffer, &srcPitch))) {
			LOG_ERR("Nvmedia image get bits fail %d\n", status);
			goto fail;
		}

		//GetTimeMicroSec(&tend);
		//NvU64 td = tend - tbegin;
		//LOG_ERR("%u\n", td);
		//sum_time += td*1000;
		//num_f++;

		/* ***** END TIMING ***** */ 
		//clock_gettime(CLOCK_MONOTONIC, &tpend);
		//printf("%ld %ld\n", tpend.tv_sec, tpend.tv_nsec);

		//diff_sec = tpend.tv_sec - tpstart.tv_sec;
		//diff_nsec = tpend.tv_nsec - tpstart.tv_nsec; 
		//float total_nsec = diff_sec * 1000000000 + diff_nsec;
		//if(total_nsec > max_time)
		//	max_time = total_nsec;
		//sum_time += total_nsec;
		//num_f++;
		/* ***** END TIMING ***** */

		NvMediaImageUnlock(image);
#if 0
		BufferPool_ReleaseBuffer(display->outputBuffersPool, outputImageBuffer);
#endif
		NvMediaEglStreamConsumerReleaseImage(display->consumer, image);
	}

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
                       EGLDisplay eglDisplay, EGLStreamKHR eglStream
                       )
{
    display->consumerDone = consumerDone;
    display->metadataEnable = NV_FALSE;
    display->eglDisplay = eglDisplay;
    display->eglStream = eglStream;
    display->encodeEnable = NV_FALSE;
	display->outputBuffersPool = NULL;
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
    display->consumer = NvMediaEglStreamConsumerCreate(
        display->device,
        display->eglDisplay,
        display->eglStream,
        0x89);
    if(!display->consumer) {
        LOG_ERR("image_display_init: Unable to create consumer\n");
        return NV_FALSE;
    }

	g_display = display;

#if 0
    if (IsFailed(NvThreadCreate(&display->procThread, &procThreadFunc, (void *)display, NV_THREAD_PRIORITY_NORMAL))) {
        LOG_ERR("Nvmedia image consumer init: Unable to create process thread\n");
        display->procThreadExited = NV_TRUE;
        return NV_FALSE;
    }
#endif
	display->quit = NV_FALSE;
	g_display = malloc(sizeof(test_nvmedia_consumer_display_s));
	memcpy(g_display, display, sizeof(test_nvmedia_consumer_display_s));
	LOG_ERR("%s %d %d\n", __func__, __LINE__, g_display->quit);
    return NV_TRUE;
}

void image_display_Deinit(test_nvmedia_consumer_display_s *display)
{
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
        display->consumer = NULL;
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
        buffer = (*releaseList)->tag;
        BufferPool_ReleaseBuffer(buffer->bufferPool, buffer);
        *releaseList = NULL;
        releaseList++;
    }
}