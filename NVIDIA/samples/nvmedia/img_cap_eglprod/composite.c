/* Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <limits.h>
#include <math.h>

#include "composite.h"
#include "save.h"
#include "nvmimg_producer.h"

static NvMediaStatus
_CreateImageQueue(
                  NvMediaDevice *device,
                  NvQueue **queue,
                  uint32_t queueSize,
                  uint32_t width,
                  uint32_t height,
                  NvMediaSurfaceType surfType,
                  NvMediaSurfAllocAttr *surfAllocAttrs,
                  uint32_t numSurfAllocAttrs,
                  NvMediaImage **imagesAllocCtx)
{
    uint32_t j = 0;
    NvMediaImage *image = NULL;
    NvMediaStatus status = NVMEDIA_STATUS_OK;

    if (NvQueueCreate(queue,
                      queueSize,
                      sizeof(NvMediaImage *)) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to create image Queue \n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        return status;
    }

    for (j = 0; j < queueSize; j++) {
        LOG_DBG("%s: NvMediaImageCreateNew\n", __func__);
        image =  NvMediaImageCreateNew(device,          /* device                  */
                                    surfType,           /* NvMediaSurfaceType type */
                                    surfAllocAttrs,     /* surf allocation attrs   */
                                    numSurfAllocAttrs,  /* num attrs               */
                                    0);                 /* flags                   */
        if (!image) {
            LOG_ERR("%s: NvMediaImageCreate failed for image %d",
                        __func__, j);
            status = NVMEDIA_STATUS_ERROR;
            return status;
        }

        image->tag = *queue;

        if (IsFailed(NvQueuePut(*queue,
                                (void *)&image,
                                NV_TIMEOUT_INFINITE))) {
            LOG_ERR("%s: Pushing image to image queue failed\n", __func__);
            status = NVMEDIA_STATUS_ERROR;
            return status;
        }

        /* maintain a local context of the allocated images*/
        imagesAllocCtx[j] = image;
    }

    return NVMEDIA_STATUS_OK;
}

static uint32_t
_CompositeThreadFunc(
            void *data)
{
    NvCompositeContext *compCtx= (NvCompositeContext *)data;
    NvMediaImage *imageIn = NULL;
    NvMediaImage *compImage = NULL;
    uint32_t i = 0;
    NvMediaStatus status;

    while (!(*compCtx->quit)) {
        /* Acquire all the images from capture queues */
        imageIn = NULL;
        while (NvQueueGet(compCtx->inputQueue,
                        (void *)&imageIn,
                        COMPOSITE_DEQUEUE_TIMEOUT) != NVMEDIA_STATUS_OK) {
            LOG_DBG("%s: Waiting for input image from queue %d\n", __func__, i);
            if (*compCtx->quit) {
                goto loop_done;
            }
        }

        if (NVMEDIA_TRUE == compCtx->isRgba) {
            /* Acquire image for storing composited images */
            while (NvQueueGet(compCtx->compositeQueue,
                          (void *)&compImage,
                          COMPOSITE_DEQUEUE_TIMEOUT) != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: compositeQueue is empty\n", __func__);
                if (*compCtx->quit)
                    goto loop_done;
            }

            /* Blit for color conversion and flipping */
            if(NVMEDIA_TRUE == compCtx->flipY) {
                compCtx->blitParams.validFields |= NVMEDIA_2D_BLIT_PARAMS_DST_TRANSFORM;
                compCtx->blitParams.dstTransform = NVMEDIA_TRANSFORM_FLIP_VERTICAL;
            }

            status = NvMedia2DBlitEx(compCtx->i2d,
                                compImage,
                                NULL,
                                imageIn,
                                NULL,
                                &compCtx->blitParams,
                                NULL);
            if (status != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: NvMedia2DBlitEx failed\n", __func__);
                *compCtx->quit = NVMEDIA_TRUE;
                goto loop_done;
            }

            /* Put composited image onto output queue */
            while (NvQueuePut(compCtx->outputQueue,
                          (void *)&(compImage),
                          COMPOSITE_ENQUEUE_TIMEOUT) != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: Waiting to acquire bufffer\n", __func__);
                if (*compCtx->quit)
                    goto loop_done;
            }
            compImage = NULL;
        } else {
            while (NvQueuePut(compCtx->outputQueue,
                          (void *)&(imageIn),
                          COMPOSITE_ENQUEUE_TIMEOUT) != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s: Waiting to acquire bufffer\n", __func__);
            if (*compCtx->quit)
                goto loop_done;
            }
            imageIn = NULL;
        }
    loop_done:
        if (imageIn) {
            if (NvQueuePut((NvQueue *)imageIn->tag,
                            (void *)&imageIn,
                            0) != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: Failed to put the image back to queue\n", __func__);
            }
        }
        imageIn = NULL;

        if (compImage) {
            if (NvQueuePut((NvQueue *) compImage->tag,
                           (void *) &compImage,
                           0) != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: Failed to put the image back to compositeQueue\n", __func__);
            }
            compImage = NULL;
        }
    }
    LOG_INFO("%s: Composite thread exited\n", __func__);
    compCtx->exitedFlag = NVMEDIA_TRUE;
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
CompositeInit(
        NvMainContext *mainCtx)
{
    NvCompositeContext *compCtx  = NULL;
    NvSaveContext   *saveCtx = NULL;
    TestArgs           *testArgs = mainCtx->testArgs;
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;
    uint32_t width = 0, height = 0;
    NvMediaSurfAllocAttr surfAllocAttrs[8];
    uint32_t numSurfAllocAttrs;

    /* allocating Composite context */
    mainCtx->ctxs[COMPOSITE_ELEMENT]= malloc(sizeof(NvCompositeContext));
    if (!mainCtx->ctxs[COMPOSITE_ELEMENT]){
        LOG_ERR("%s: Failed to allocate memory for composite context\n", __func__);
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    compCtx = mainCtx->ctxs[COMPOSITE_ELEMENT];
    memset(compCtx,0,sizeof(NvCompositeContext));
    saveCtx = mainCtx->ctxs[SAVE_ELEMENT];

    /* initialize context */
    compCtx->quit      =  &mainCtx->quit;
    compCtx->flipY = testArgs->flipY;
    compCtx->consumervm = testArgs->consumervm;
    compCtx->exitedFlag = NVMEDIA_TRUE;
    compCtx->isRgba = testArgs->isRgba;
    /* Create NvMedia Device */
    if (NVMEDIA_TRUE == compCtx->isRgba) {
        compCtx->device = NvMediaDeviceCreate();
        if (!compCtx->device) {
            status = NVMEDIA_STATUS_ERROR;
            LOG_ERR("%s: Failed to create NvMedia device\n", __func__);
            goto failed;
        }

        /* create i2d handle */
        compCtx->i2d = NvMedia2DCreate(compCtx->device);
        if (!compCtx->i2d) {
            LOG_ERR("%s: Failed to create NvMedia 2D i2d\n", __func__);
            status = NVMEDIA_STATUS_ERROR;
            goto failed;
        }
    }

    /* Create input Queues */
    if (NvQueueCreate(&compCtx->inputQueue,
            COMPOSITE_QUEUE_SIZE,
            sizeof(NvMediaImage *)) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to create Composite inputQueue\n",
                __func__);
        status = NVMEDIA_STATUS_ERROR;
        goto failed;
    }

    /* Create Output queue */
    if (IsFailed(NvQueueCreate(&compCtx->outputQueue,
                               COMPOSITE_QUEUE_SIZE,
                               sizeof(NvMediaImage *)))) {
        LOG_ERR("%s: Failed to create queue\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        goto failed;
    }


    if (NVMEDIA_TRUE == compCtx->isRgba) {
        /* Get the output width and height for composition */
        width = saveCtx->threadCtx.width;
        height = saveCtx->threadCtx.height;
        compCtx->width = saveCtx->threadCtx.width;
        compCtx->height = saveCtx->threadCtx.height;

        /* Create compositeQueue for storing composited Images */
        surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
        surfAllocAttrs[0].value = width;
        surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
        surfAllocAttrs[1].value = height;
        surfAllocAttrs[2].type = NVM_SURF_ATTR_CPU_ACCESS;
        //surfAllocAttrs[2].value = NVM_SURF_ATTR_CPU_ACCESS_UNCACHED;
        surfAllocAttrs[2].value = NVM_SURF_ATTR_CPU_ACCESS_CACHED;
        surfAllocAttrs[3].type = NVM_SURF_ATTR_ALLOC_TYPE;
        surfAllocAttrs[3].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
        numSurfAllocAttrs = 4;

        if (testArgs->cross == STANDALONE_CROSS_PART) {
            surfAllocAttrs[4].type = NVM_SURF_ATTR_PEER_VM_ID;
            surfAllocAttrs[4].value = compCtx->consumervm;
            numSurfAllocAttrs += 1;
        }

        NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);
        NVM_SURF_FMT_SET_ATTR_RGBA(surfFormatAttrs,RGBA,UINT,8,BL);

        status = _CreateImageQueue(compCtx->device,
                               &compCtx->compositeQueue,
                               COMPOSITE_QUEUE_SIZE,
                               width,
                               height,
                               NvMediaSurfaceFormatGetType(surfFormatAttrs, NVM_SURF_FMT_ATTR_MAX),
                               surfAllocAttrs,
                               numSurfAllocAttrs,
                               compCtx->images);
        if (status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s: compositeQueue creation failed\n", __func__);
            goto failed;
        }

        LOG_DBG("%s: Composite Queue: %ux%u, images: %u \n",
            __func__, width, height, COMPOSITE_QUEUE_SIZE);
    }
    return NVMEDIA_STATUS_OK;
failed:
    LOG_ERR("%s: Failed to initialize Composite\n", __func__);
    return status;
}

NvMediaStatus
CompositeFini(
    NvMainContext *mainCtx)
{
    NvCompositeContext *compCtx = NULL;
    NvMediaImage *image = NULL;
    NvMediaStatus status;
    uint32_t j;

    if (!mainCtx)
        return NVMEDIA_STATUS_OK;

    compCtx = mainCtx->ctxs[COMPOSITE_ELEMENT];
    if (!compCtx)
        return NVMEDIA_STATUS_OK;

    /* Wait for composite thread to exit */
    while (!compCtx->exitedFlag) {
        LOG_DBG("%s: Waiting for composite thread to quit\n", __func__);
    }

    *compCtx->quit = NVMEDIA_TRUE;

    /* Destroy thread */
    if (compCtx->compositeThread) {
        status = NvThreadDestroy(compCtx->compositeThread);
        if (status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s: Failed to destroy composite thread \n", __func__);
        }
    }

    /*Flush and destroy the input queues*/
    if (compCtx->inputQueue) {
        LOG_DBG("%s: Flushing the composite input queue", __func__);
        while (IsSucceed(NvQueueGet(compCtx->inputQueue,
                                    &image,
                                    COMPOSITE_DEQUEUE_TIMEOUT))) {
            if (image) {
                if (NvQueuePut((NvQueue *)image->tag,
                                (void *)&image,
                                0) != NVMEDIA_STATUS_OK) {
                    LOG_ERR("%s: Failed to put image back in queue\n", __func__);
                    break;
                }
            }
            image=NULL;
        }
        NvQueueDestroy(compCtx->inputQueue);
    }

    /*Flush and destroy the output queue*/
    if (compCtx->outputQueue) {
        LOG_DBG("%s: Flushing the composite output queue\n", __func__);
        while (IsSucceed(NvQueueGet(compCtx->outputQueue, &image, 0))) {
            if (image) {
                if (NvQueuePut((NvQueue *)image->tag,
                               (void *)&image,
                               0) != NVMEDIA_STATUS_OK) {
                    LOG_ERR("%s: Failed to put image back in queue\n", __func__);
                    break;
                }
                image = NULL;
            }
        }
        NvQueueDestroy(compCtx->outputQueue);
    }
    image = NULL;

    if (NVMEDIA_TRUE == compCtx->isRgba) {
        /* Destroy the compositeQueue */
        if (compCtx->compositeQueue) {
            while ((NvQueueGet(compCtx->compositeQueue,
                           &image,
                           0)) == NVMEDIA_STATUS_OK) {
            /* Only flush here */
            /* Destroy the images queued by local context */
            }
            LOG_DBG("%s: Destroying CompositeQueue \n", __func__);
            NvQueueDestroy(compCtx->compositeQueue);
        }

        /* Destroy the allocated NvMedia Images */
        for(j = 0; j < COMPOSITE_QUEUE_SIZE; j++)
        {
            image = compCtx->images[j];
            if (image) {
                NvMediaImageDestroy(image);
                image = NULL;
            }
        }

        if (compCtx->i2d)
            NvMedia2DDestroy(compCtx->i2d);

        if (compCtx->device)
            NvMediaDeviceDestroy(compCtx->device);
    }

    if (compCtx)
        free(compCtx);

    LOG_INFO("%s: CompositeFini done\n", __func__);
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
CompositeProc(
    NvMainContext *mainCtx)
{
    NvCompositeContext *compCtx = NULL;
    NvMediaStatus status = NVMEDIA_STATUS_OK;

    if (!mainCtx) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }
    compCtx = mainCtx->ctxs[COMPOSITE_ELEMENT];

    /* Create thread to blit images */
    compCtx->exitedFlag = NVMEDIA_FALSE;
    status = NvThreadCreate(&compCtx->compositeThread,
                                &_CompositeThreadFunc,
                                (void *)compCtx,
                                NV_THREAD_PRIORITY_NORMAL);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to create composite Thread\n",
                __func__);
        compCtx->exitedFlag = NVMEDIA_TRUE;
    }

    return status;
}
