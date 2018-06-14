/*
 * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

/*
 * DESCRIPTION:  Simple EGL stream producer for Image2D
 */

#include "nvmimg_producer.h"
#include "main.h"
#include "thread_utils.h"
#include "composite.h"
#include "eglstrm_setup.h"

#define FRAMES_PER_SECOND      30

#if defined(EXTENSION_LIST)
EXTENSION_LIST(EXTLST_EXTERN)
#endif

//this is used to identify if we are initializing the first eglstream or not
static int first_time = 0;

static uint32_t
_ProducerThreadFunc(
        void* data)
{
    NvEglStreamContext *producerCtx = (NvEglStreamContext *)data;
    NvMediaImage *image = NULL;
    NvMediaImage *releaseImage = NULL;
    NvMediaStatus status;

    while (!(*producerCtx->quit)) {

		image = NULL;
		while (NvQueueGet(producerCtx->inputQueue, &image, EGL_DEQUEUE_TIMEOUT) !=
				NVMEDIA_STATUS_OK) {
			LOG_ERR("%s: Producer input queue empty\n", __func__);
			if (*producerCtx->quit)
				goto loop_done;
		}

		uint32_t* pTag = (uint32_t*)(&image->tag);
		//use the last two bits to indicate whether this buffer is still used by other application or not (right now only two applications)
		*pTag += 2;

		/* Post outputImage to egl-stream */
        status = NvMediaEglStreamProducerPostImage(producerCtx->eglProducer,
                        image,
                        NULL);

        if(status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s: NvMediaEglStreamProducerPostImage failed\n", __func__);
            goto loop_done;
        }

        /* Post outputImage to egl-stream */
        status = NvMediaEglStreamProducerPostImage(producerCtx->eglProducer1,
                        image,
                        NULL);

        if(status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s: NvMediaEglStreamProducerPostImage failed\n", __func__);
            goto loop_done;
        }


        /* Get back from the egl-stream */
        status = NvMediaEglStreamProducerGetImage(producerCtx->eglProducer,
                                    &releaseImage,
                                    0);


		if (status == NVMEDIA_STATUS_OK) {
			LOG_DBG("%s %d: %p\n", __func__, __LINE__, releaseImage->tag);
			pTag = (uint32_t*)(&releaseImage->tag);
			*pTag -= 1;
			if ((*pTag & 0x01) == 0) {
				/* Return the image back to the bufferpool */
				if (NvQueuePut((NvQueue *)releaseImage->tag,
							(void *)&releaseImage,
							0) != NVMEDIA_STATUS_OK) {
					LOG_ERR("%s: Failed to put image back in queue\n", __func__);
					*producerCtx->quit = NVMEDIA_TRUE;
					goto loop_done;
				}
			}
		} else {
			LOG_DBG ("%s: NvMediaEglStreamProducerGetImage waiting\n", __func__);
			continue;
		}

		/* Get back from the egl-stream */
        status = NvMediaEglStreamProducerGetImage(producerCtx->eglProducer1,
                                    &releaseImage,
                                    0);

		if (status == NVMEDIA_STATUS_OK) {
			LOG_DBG("%s %d: %p\n", __func__, __LINE__, releaseImage->tag);
			pTag = (uint32_t*)(&releaseImage->tag);
			*pTag -= 1;
			if ((*pTag & 0x01) == 0) {
				/* Return the image back to the bufferpool */
				if (NvQueuePut((NvQueue *)releaseImage->tag,
							(void *)&releaseImage,
							0) != NVMEDIA_STATUS_OK) {
					LOG_ERR("%s: Failed to put image back in queue\n", __func__);
					*producerCtx->quit = NVMEDIA_TRUE;
					goto loop_done;
				}
			}
		} else {
			LOG_DBG ("%s: NvMediaEglStreamProducerGetImage waiting\n", __func__);
			continue;
		}

        image = NULL;
        releaseImage = NULL;
    }

loop_done:

    if (image) {
        if (NvQueuePut((NvQueue *)image->tag,
                           (void *)&image,
                           0) != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s: Failed to put image back in queue\n", __func__);
            *producerCtx->quit = NVMEDIA_TRUE;
        }
            image = NULL;
    }

    LOG_INFO("%s: Display thread exited\n", __func__);
    producerCtx->exitedFlag = NVMEDIA_TRUE;
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
EglProducerInit(
        NvMainContext *mainCtx)
{
	NvMediaStatus status = NVMEDIA_STATUS_OK;
	if (first_time == 0) {
		LOG_DBG("%s %d: eglstream0\n", __func__, __LINE__);
		NvEglStreamContext   *ProducerCtx  = NULL;
		TestArgs           *testArgs = mainCtx->testArgs;

		/* Allocating EGL Stream Producer context */
		mainCtx->ctxs[EGL_ELEMENT]= malloc(sizeof(NvEglStreamContext));
		if (!mainCtx->ctxs[EGL_ELEMENT]){
			LOG_ERR("%s: Failed to allocate memory for EGL context\n", __func__);
			return NVMEDIA_STATUS_OUT_OF_MEMORY;
		}

		ProducerCtx = mainCtx->ctxs[EGL_ELEMENT];
		memset(ProducerCtx,0,sizeof(NvEglStreamContext));

		EGLStreamKHR eglStream = EGL_NO_STREAM_KHR;
		EGLDisplay eglDisplay = EGL_NO_DISPLAY;
		eglDisplay = EGLDefaultDisplayInit();

		if (EGL_NO_DISPLAY == eglDisplay) {
			LOG_ERR("main - failed to initialize egl \n");
			status = NVMEDIA_STATUS_ERROR;
			goto fail;
		}

		eglStream = EGLStreamInit(eglDisplay, testArgs, SOCK_PATH);
		if(!eglStream) {
			LOG_ERR("main - failed to initialize eglst \n");
			status = NVMEDIA_STATUS_ERROR;
			goto fail;
		}

		ProducerCtx->eglStream = eglStream;
		ProducerCtx->eglDisplay = eglDisplay;
		ProducerCtx->quit = &mainCtx->quit;
	} else {
		LOG_DBG("%s %d: eglstream1\n", __func__, __LINE__);
		NvEglStreamContext   *ProducerCtx  = mainCtx->ctxs[EGL_ELEMENT];
		TestArgs           *testArgs = mainCtx->testArgs;
		EGLStreamKHR eglStream = EGL_NO_STREAM_KHR;
		EGLDisplay eglDisplay = EGL_NO_DISPLAY;
		eglDisplay = EGLDefaultDisplayInit();

		if (EGL_NO_DISPLAY == eglDisplay) {
			LOG_ERR("main - failed to initialize egl \n");
			status = NVMEDIA_STATUS_ERROR;
			goto fail;
		}

		eglStream = EGLStreamInit(eglDisplay, testArgs, SOCK_PATH1);
		if(!eglStream) {
			LOG_ERR("main - failed to initialize eglst \n");
			status = NVMEDIA_STATUS_ERROR;
			goto fail;
		}
		ProducerCtx->eglStream1 = eglStream;
		ProducerCtx->eglDisplay1 = eglDisplay;
	}

    return status;
fail:
    EglProducerFini(mainCtx);
    return status;
}

void
EglProducerFini(
    NvMainContext *mainCtx)
{
    NvEglStreamContext   *ProducerCtx  = NULL;
    ProducerCtx = mainCtx->ctxs[EGL_ELEMENT];

    if(ProducerCtx->ProducerThread) {
        NvThreadDestroy(ProducerCtx->ProducerThread);
    }

    if(ProducerCtx->eglProducer) {
        NvMediaEglStreamProducerDestroy(ProducerCtx->eglProducer);
        ProducerCtx->eglProducer = NULL;
    }

    EGLStreamFini(ProducerCtx->eglDisplay, ProducerCtx->eglStream);


    EGLDefaultDisplayDeinit(ProducerCtx->eglDisplay);
    free(ProducerCtx);
}


NvMediaStatus
EglProducerProc(
        NvMainContext *mainCtx)
{
	NvMediaStatus status = NVMEDIA_STATUS_OK;
	EGLint streamState = 0;
	NvEglStreamContext   *ProducerCtx  = NULL;
	NvCompositeContext *compCtx  = NULL;
	NvMediaSurfaceType prodSurfaceType;
	if (first_time == 0) {
		LOG_DBG("%s %d: producer0\n", __func__, __LINE__);
		compCtx = mainCtx->ctxs[COMPOSITE_ELEMENT];
		ProducerCtx = mainCtx->ctxs[EGL_ELEMENT];
		TestArgs *testArgs = mainCtx->testArgs;
		NVM_SURF_FMT_DEFINE_ATTR(prodSurfFormatAttrs);

		if (testArgs->isRgba == NVMEDIA_TRUE) {
			NVM_SURF_FMT_SET_ATTR_RGBA(prodSurfFormatAttrs,RGBA,UINT,8,PL);
		} else {
			NVM_SURF_FMT_SET_ATTR_YUV(prodSurfFormatAttrs,YUV,422,PLANAR,UINT,8,PL);
		}

		if (!mainCtx) {
			LOG_ERR("%s: Bad parameter\n", __func__);
			status = NVMEDIA_STATUS_ERROR;
			goto fail;
		}

		prodSurfaceType = NvMediaSurfaceFormatGetType(prodSurfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);

		while(streamState != EGL_STREAM_STATE_CONNECTING_KHR) {
			LOG_ERR("%s %d: streamState %d\n", __func__, __LINE__, streamState);
			if(!eglQueryStreamKHRfp(
						ProducerCtx->eglDisplay,
						ProducerCtx->eglStream,
						EGL_STREAM_STATE_KHR,
						&streamState)) {
				LOG_ERR("main: before init producer, eglquerystreamkhr EGL_STREAM_STATE_KHR"\
						"failed\n");
				status = NVMEDIA_STATUS_ERROR;
				goto fail;
			}
		}

		ProducerCtx->eglProducer = NvMediaEglStreamProducerCreate(compCtx->device,
				ProducerCtx->eglDisplay,
				ProducerCtx->eglStream,
				prodSurfaceType,
				compCtx->width,
				compCtx->height);
		if(!ProducerCtx->eglProducer) {
			LOG_ERR("%s: failed to create egl producer\n", __func__);
			status = NVMEDIA_STATUS_ERROR;
			goto fail;
		}

		//set multiSend according to NV's document
		NvMediaEglStreamProducerAttributes producerAttributes = { .multiSend = NVMEDIA_TRUE};
		NvMediaEglStreamProducerSetAttributes(ProducerCtx->eglProducer, NVMEDIA_EGL_STREAM_PRODUCER_ATTRIBUTE_MULTISEND, &producerAttributes);

		//use the same queue
		ProducerCtx->inputQueue = compCtx->outputQueue;

		first_time++;

	} else {
		LOG_DBG("%s %d: producer1\n", __func__, __LINE__);
		TestArgs *testArgs = mainCtx->testArgs;
		compCtx = mainCtx->ctxs[COMPOSITE_ELEMENT];
		ProducerCtx = mainCtx->ctxs[EGL_ELEMENT];
		NVM_SURF_FMT_DEFINE_ATTR(prodSurfFormatAttrs);

		if (testArgs->isRgba == NVMEDIA_TRUE) {
			NVM_SURF_FMT_SET_ATTR_RGBA(prodSurfFormatAttrs,RGBA,UINT,8,PL);
		} else {
			NVM_SURF_FMT_SET_ATTR_YUV(prodSurfFormatAttrs,YUV,422,PLANAR,UINT,8,PL);
		}
		prodSurfaceType = NvMediaSurfaceFormatGetType(prodSurfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);
		
		while(streamState != EGL_STREAM_STATE_CONNECTING_KHR) {
			if(!eglQueryStreamKHRfp(
						ProducerCtx->eglDisplay1,
						ProducerCtx->eglStream1,
						EGL_STREAM_STATE_KHR,
						&streamState)) {
				LOG_ERR("main: before init producer, eglQueryStreamKHRfp EGL_STREAM_STATE_KHR"\
						"failed\n");
				status = NVMEDIA_STATUS_ERROR;
				goto fail;
			}
		}

		ProducerCtx->eglProducer1 = NvMediaEglStreamProducerCreate(compCtx->device,
				ProducerCtx->eglDisplay1,
				ProducerCtx->eglStream1,
				prodSurfaceType,
				compCtx->width,
				compCtx->height);
		if(!ProducerCtx->eglProducer1) {
			LOG_ERR("%s: failed to create egl producer1\n", __func__);
			status = NVMEDIA_STATUS_ERROR;
			goto fail;
		}

		//set multiSend according to NV's document
		NvMediaEglStreamProducerAttributes producerAttributes = { .multiSend = NVMEDIA_TRUE};
		NvMediaEglStreamProducerSetAttributes(ProducerCtx->eglProducer1, NVMEDIA_EGL_STREAM_PRODUCER_ATTRIBUTE_MULTISEND, &producerAttributes);

		//create thread when we connect two eglstreams
		/* Create thread for EGL producer Stream */
		status = NvThreadCreate(&ProducerCtx->ProducerThread,
				&_ProducerThreadFunc,
				(void *)ProducerCtx,
				NV_THREAD_PRIORITY_NORMAL);
		if (status != NVMEDIA_STATUS_OK) {
			LOG_ERR("%s: Failed to create producer Thread\n",
					__func__);
			ProducerCtx->exitedFlag = NVMEDIA_TRUE;
			status = NVMEDIA_STATUS_ERROR;
			goto fail;
		}
	}

    return status;
fail:
    EglProducerFini(mainCtx);
    return status;
}
