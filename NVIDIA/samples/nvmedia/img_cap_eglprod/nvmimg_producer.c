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

        /* Post outputImage to egl-stream */
        status = NvMediaEglStreamProducerPostImage(producerCtx->eglProducer,
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
            /* Return the image back to the bufferpool */
            if (NvQueuePut((NvQueue *)releaseImage->tag,
                                (void *)&releaseImage,
                                    0) != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: Failed to put image back in queue\n", __func__);
                *producerCtx->quit = NVMEDIA_TRUE;
                goto loop_done;
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

    eglStream = EGLStreamInit(eglDisplay, testArgs);
    if(!eglStream) {
        LOG_ERR("main - failed to initialize eglst \n");
        status = NVMEDIA_STATUS_ERROR;
        goto fail;
    }

    ProducerCtx->eglStream = eglStream;
    ProducerCtx->eglDisplay = eglDisplay;
    ProducerCtx->quit = &mainCtx->quit;

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
    compCtx = mainCtx->ctxs[COMPOSITE_ELEMENT];
    ProducerCtx = mainCtx->ctxs[EGL_ELEMENT];
    NvMediaSurfaceType prodSurfaceType;
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
        if(!eglQueryStreamKHRfp(
            ProducerCtx->eglDisplay,
            ProducerCtx->eglStream,
            EGL_STREAM_STATE_KHR,
            &streamState)) {
                LOG_ERR("main: before init producer, eglQueryStreamKHR EGL_STREAM_STATE_KHR"\
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
        LOG_ERR("%s: Failed to create EGL producer\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        goto fail;
    }

    ProducerCtx->inputQueue = compCtx->outputQueue;

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

    return status;
fail:
    EglProducerFini(mainCtx);
    return status;
}
