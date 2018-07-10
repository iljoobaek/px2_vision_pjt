/* Copyright (c) 2015-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <limits.h>
#include <math.h>

#include "capture.h"
#include "process2d.h"
#include "interop.h"

static NvMediaStatus
SetProcess2DParams (
        NvProcess2DContext *ctx)
{
    NvMedia2DBlitParameters *params = ctx->blitParams;

    params->validFields = 0;
    params->flags = 0;
    params->filter = 1;
    params->validFields = 0;
    params->dstTransform = 0;
    params->colorStandard = 0;

    /* Default values in the config file for full resolution */
    if(ctx->srcRect->x0 == 0 &&
        ctx->srcRect->y0 == 0 &&
        ctx->srcRect->x1 == 0 &&
        ctx->srcRect->y1 == 0) {

        ctx->dstRect->x0  = ctx->srcRect->x0 = 0;
        ctx->dstRect->x1  = ctx->srcRect->x1 = ctx->width;
        ctx->dstRect->y0  = ctx->srcRect->y0 = 0;
        ctx->dstRect->y1  = ctx->srcRect->y1 = ctx->height;
    }

    return NVMEDIA_STATUS_OK;
}

NvMediaStatus Process2DInit (IPPCtx *ippCtx)
{
    NvProcess2DContext *proc2DCtx  = NULL;
    NvMediaImage *image = NULL;
    NvU32 i;
    char *env;

    /* allocating Process2D context */
    mainCtx->ctxs[PROCESS_2D]= malloc(sizeof(NvProcess2DContext));
    if(!mainCtx->ctxs[PROCESS_2D]){
        LOG_ERR("%s: Failed to allocate memory for process2D context\n", __func__);
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    proc2DCtx = mainCtx->ctxs[PROCESS_2D];
    memset(proc2DCtx,0,sizeof(NvProcess2DContext));

    /* initialize context */
    proc2DCtx->width = mainCtx->width;
    proc2DCtx->height = mainCtx->height;
    interopCtx->quit   = &ippCtx->quit;
    proc2DCtx->device = ippCtx->device;

    NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);
    /* set output surface formats */
    if(!strcasecmp(mainCtx->surfFmt,"420p")) {
        NVM_SURF_FMT_SET_ATTR_YUV(surfFormatAttrs,YUV,420,SEMI_PLANAR,UINT,8,PL);
    } else if(!strcasecmp(mainCtx->surfFmt,"rgba")) {
        NVM_SURF_FMT_SET_ATTR_RGBA(surfFormatAttrs,RGBA,UINT,8,PL);
    } else {
        LOG_ERR("Unrecognized output surf fmt: Using RGBA as output format\n");
        NVM_SURF_FMT_SET_ATTR_RGBA(surfFormatAttrs,RGBA,UINT,8,PL);
    }
    proc2DCtx->outputSurfType = NvMediaSurfaceFormatGetType(surfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);

    proc2DCtx->i2d = NvMedia2DCreate(proc2DCtx->device);
    if(!proc2DCtx->i2d) {
        LOG_ERR("%s: Failed to create NvMedia 2D i2d\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }

    proc2DCtx->blitParams = calloc(1, sizeof(NvMedia2DBlitParameters));
    if(!proc2DCtx->blitParams) {
        LOG_ERR("%s: Out of memory", __func__);
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    proc2DCtx->srcRect = malloc(sizeof(NvMediaRect));
    proc2DCtx->dstRect = malloc(sizeof(NvMediaRect));
    if(!proc2DCtx->srcRect || !proc2DCtx->dstRect) {
        LOG_ERR("%s: Out of memory", __func__);
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    memset(proc2DCtx->srcRect,0,sizeof(NvMediaRect));
    memset(proc2DCtx->dstRect,0,sizeof(NvMediaRect));
    if(IsFailed(SetProcess2DParams(proc2DCtx))) {
        return NVMEDIA_STATUS_ERROR;
    }

    /*Create Process2D input Queue*/
    if(IsFailed(NvQueueCreate(&proc2DCtx->inputQueue,
                              IMAGE_BUFFERS_POOL_SIZE,
                              sizeof(NvMediaImage *)))) {
        return NVMEDIA_STATUS_ERROR;
    }

    /*Create Process2D processing Queue*/
     if(IsFailed(NvQueueCreate(&proc2DCtx->processQueue,
                               IMAGE_BUFFERS_POOL_SIZE,
                               sizeof(NvMediaImage *)))) {
         return NVMEDIA_STATUS_ERROR;
     }

     proc2DCtx->surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
     proc2DCtx->surfAllocAttrs[0].value = proc2DCtx->width;
     proc2DCtx->surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
     proc2DCtx->surfAllocAttrs[1].value = proc2DCtx->height;
     proc2DCtx->surfAllocAttrs[2].type = NVM_SURF_ATTR_ALLOC_TYPE;
     proc2DCtx->surfAllocAttrs[2].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
     proc2DCtx->numSurfAllocAttrs = 3;
     if ((env = getenv("DISPLAY_VM"))) {
         proc2DCtx->surfAllocAttrs[3].type = NVM_SURF_ATTR_PEER_VM_ID;
         proc2DCtx->surfAllocAttrs[3].value = atoi(env);
         proc2DCtx->numSurfAllocAttrs += 1;
     }

     for(i = 0; i < IMAGE_BUFFERS_POOL_SIZE; i++) {
         image = NvMediaImageCreateNew(proc2DCtx->device,               // device
                                      proc2DCtx->outputSurfType,        // surface type
                                      proc2DCtx->surfAllocAttrs,       // surf alloc attrs
                                      proc2DCtx->numSurfAllocAttrs,    // num surf alloc attrs
                                      0);  // attributes
         if(!image) {
              return NVMEDIA_STATUS_ERROR;
         }
         image->tag = proc2DCtx->processQueue;

         if(IsFailed(NvQueuePut(proc2DCtx->processQueue,
                 (void *)&image, NV_TIMEOUT_INFINITE)))
             return NVMEDIA_STATUS_ERROR;
     }

     LOG_DBG("%s: Process Queue: %ux%u, images: %u \n",
         __func__, proc2DCtx->width, proc2DCtx->height,
         IMAGE_BUFFERS_POOL_SIZE);

    return NVMEDIA_STATUS_OK;
}

NvMediaStatus Process2DFini (NvMainContext *mainCtx)
{
    NvProcess2DContext *proc2DCtx = NULL;
    proc2DCtx = mainCtx->ctxs[PROCESS_2D];
    /*Flush the input queue*/
    NvMediaImage *image = NULL;

    if(proc2DCtx->inputQueue) {
        while(IsSucceed(NvQueueGet(proc2DCtx->inputQueue, &image, 0))) {
            if (image) {
                NvQueuePut((NvQueue *)image->tag,
                       (void *)&image,
                       ENQUEUE_TIMEOUT);
                image = NULL;
            }
        }
        LOG_DBG("\n Flushing the process2D input queue");
        NvQueueDestroy(proc2DCtx->inputQueue);
    }

    if(proc2DCtx->processQueue) {
        while(IsSucceed(NvQueueGet(proc2DCtx->processQueue, &image, 0))) {
            if (image) {
                NvMediaImageDestroy(image);
                image = NULL;
            }
        }
        LOG_DBG("\n Destroying process2D process queue");
        NvQueueDestroy(proc2DCtx->processQueue);
    }

    if(proc2DCtx->i2d)
        NvMedia2DDestroy(proc2DCtx->i2d);
    if(proc2DCtx->dstRect)
        free(proc2DCtx->dstRect);
    if(proc2DCtx->srcRect)
        free(proc2DCtx->srcRect);
    if(proc2DCtx->blitParams)
        free(proc2DCtx->blitParams);
    if(proc2DCtx)
        free(proc2DCtx);

    return NVMEDIA_STATUS_OK;
}


void Process2DProc (void* data,void* user_data)
{
    NvProcess2DContext *proc2DCtx = NULL;
    NvInteropContext   *interopCtx = NULL;
    NvMediaImage *capturedImage = NULL;
    NvMediaImage *processedImage = NULL;
    NvU32 framecount = 0;

    NvMainContext *mainCtx = (NvMainContext *) data;
    if(!mainCtx) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return;
    }
    proc2DCtx = mainCtx->ctxs[PROCESS_2D];
    interopCtx = mainCtx->ctxs[INTEROP];

    /*Setting the queues*/
    proc2DCtx->outputQueue = interopCtx->inputQueue;

    LOG_INFO(" 2d processing thread is active\n");
    while(!(*proc2DCtx->quit)) {

        capturedImage = NULL;
        /* Get captured frame from captured frames queue */
        if(IsFailed(NvQueueGet(proc2DCtx->inputQueue,
                               (void *)&capturedImage,
                               DEQUEUE_TIMEOUT))) {
            LOG_INFO("%s: Process 2D input queue empty\n", __func__);
            goto loop_done;
        }

        framecount++;

        /* Acquire image for storing processed images */
        if(IsFailed(NvQueueGet(proc2DCtx->processQueue,
                               (void *)&processedImage,
                               DEQUEUE_TIMEOUT))) {
            LOG_ERR("%s: Process 2D process queue empty\n", __func__);
            goto loop_done;
        }

        int ret = NvMedia2DBlitEx(proc2DCtx->i2d,
                                  processedImage,
                                  proc2DCtx->dstRect,
                                  capturedImage,
                                  proc2DCtx->srcRect,
                                  proc2DCtx->blitParams,
                                  NULL);
        if(IsFailed(ret)) {
            LOG_ERR("%s: NvMedia2DBlitEx failed = %d\n", __func__,ret);
            *proc2DCtx->quit = NVMEDIA_TRUE;
            goto loop_done;
        }

        /* Put processed images on output queue */
        if(IsFailed(NvQueuePut(proc2DCtx->outputQueue,
                               (void *)&(processedImage),
                               ENQUEUE_TIMEOUT))) {
            LOG_INFO("%s: Process 2D output queue full\n", __func__);
            goto loop_done;
        }

        LOG_DBG("%s: Push image %u to Image producer \n",
                __func__, framecount);
        processedImage = NULL;
    loop_done:
        if(capturedImage) {
            NvQueuePut((NvQueue *)capturedImage->tag,
                               (void *)&capturedImage,
                               ENQUEUE_TIMEOUT);
        }
        if(processedImage) {
            NvQueuePut((NvQueue *)processedImage->tag,
                               (void *)&processedImage,
                               ENQUEUE_TIMEOUT);
        }
    }

    mainCtx->threadsExited[PROCESS_2D] = NV_TRUE;
    LOG_DBG("\n Process2D thread  finished \n");
}
