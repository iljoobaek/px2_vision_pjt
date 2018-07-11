/* Copyright (c) 2015-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "img_producer.h"
#include "buffer_utils.h"
#include "eglstrm_setup.h"

#if defined(EXTENSION_LIST)
EXTENSION_LIST(EXTLST_EXTERN)
#endif

// Print metadata information
static void PrintMetadataInfo(
    NvMediaIPPComponent *outputComponet,
    NvMediaIPPComponentOutput *output);

static void PrintMetadataInfo(
    NvMediaIPPComponent *outputComponet,
    NvMediaIPPComponentOutput *output)
{
    NvMediaIPPImageInformation imageInfo;
    NvMediaIPPPropertyControls control;
    NvMediaIPPPropertyDynamic dynamic;
    NvMediaISCEmbeddedData embeddedData;
    NvU32 topSize, bottomSize;

    if(!output || !output->metadata) {
        return;
    }

    NvMediaIPPMetadataGet(
        output->metadata,
        NVMEDIA_IPP_METADATA_IMAGE_INFO,
        &imageInfo,
        sizeof(imageInfo));

    LOG_DBG("Metadata %p: frameId %u, frame sequence #%u, cameraId %u\n",
        output->metadata, imageInfo.frameId,
        imageInfo.frameSequenceNumber, imageInfo.cameraId);

    NvMediaIPPMetadataGet(
        output->metadata,
        NVMEDIA_IPP_METADATA_CONTROL_PROPERTIES,
        &control,
        sizeof(control));

    NvMediaIPPMetadataGet(
        output->metadata,
        NVMEDIA_IPP_METADATA_DYNAMIC_PROPERTIES,
        &dynamic,
        sizeof(dynamic));

    NvMediaIPPMetadataGet(
        output->metadata,
        NVMEDIA_IPP_METADATA_EMBEDDED_DATA_ISC,
        &embeddedData,
        sizeof(embeddedData));

    LOG_DBG("Metadata %p: embedded data top (base, size) = (%#x, %u)\n",
        output->metadata,
        embeddedData.top.baseRegAddress,
        embeddedData.top.size);

    LOG_DBG("Metadata %p: embedded data bottom (base, size) = (%#x, %u)\n",
        output->metadata,
        embeddedData.bottom.baseRegAddress,
        embeddedData.bottom.size);

    topSize = NvMediaIPPMetadataGetSize(
        output->metadata,
        NVMEDIA_IPP_METADATA_EMBEDDED_DATA_TOP);

    bottomSize = NvMediaIPPMetadataGetSize(
        output->metadata,
        NVMEDIA_IPP_METADATA_EMBEDDED_DATA_BOTTOM);
    if( topSize != embeddedData.top.size ||
        bottomSize != embeddedData.bottom.size ) {
        LOG_ERR("Metadata %p: embedded data sizes mismatch\n",
            output->metadata);
    }
}

static NvMediaStatus
img2DBlitFunc(ImageProducerCtx *ctx,
               NvMediaImage *input,
               NvMediaImage *output)
{
    NvMediaImageSurfaceMap surfaceMap;

    if(!ctx || !input || !output) {
        LOG_ERR("%s: Bad parameters\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    // Make sure the image processing finished on this buffer
    if(IsSucceed(NvMediaImageLock(input,
                                  NVMEDIA_IMAGE_ACCESS_WRITE,
                                  &surfaceMap))) {
       NvMediaImageUnlock(input);
    }

    NvMutexAcquire(ctx->processorMutex);
    if(IsFailed(NvMedia2DBlitEx(ctx->i2d,
                                output,
                                NULL,
                                input,
                                NULL,
                                ctx->blitParams,
                                NULL))) {
        LOG_ERR("%s: NvMedia2DBlitEx failed\n", __func__);
        NvMutexRelease(ctx->processorMutex);
        return NVMEDIA_STATUS_ERROR;
    }

    NvMutexRelease(ctx->processorMutex);

    // Make sure the image processing finished on this buffer
    if(IsSucceed(NvMediaImageLock(output,
                                  NVMEDIA_IMAGE_ACCESS_WRITE,
                                  &surfaceMap))) {
       NvMediaImageUnlock(output);
    }

    return NVMEDIA_STATUS_OK;
}

// RAW Pipeline
static NvMediaStatus
SendIPPHumanVisionOutToEglStream(
    ImageProducerCtx *ctx,
    NvU32 ippNum,
    NvMediaIPPComponentOutput *output)
{
    NvMediaImage *retImage = NULL;
    ImageBuffer *eglBuffer = NULL, *retBuffer = NULL;
    NvMediaStatus status = NVMEDIA_STATUS_OK;
    NvU32 timeoutMS = EGL_PRODUCER_TIMEOUT_MS * ctx->ippNum;
    //int retry = EGL_PRODUCER_GET_IMAGE_MAX_RETRIES;
    ImageProducerCtx*   eglStrmProducerCtx;

    eglStrmProducerCtx = ctx;

    if(IsFailed(BufferPool_AcquireBuffer(ctx->bufferPool[ippNum],
                                          (void *)&eglBuffer))) {
        return status;
    }

    // 2D blit
    status = img2DBlitFunc(ctx,
                    output->image, // input
                    eglBuffer->image); // output
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: img2DblockFunc for IPP %d", __func__, ippNum);
        *ctx->quit = NVMEDIA_TRUE;
        return status;
    }

    // Return processed image to IPP
    status = NvMediaIPPComponentReturnOutput(ctx->outputComponent[ippNum], //component
                                             output);                //output image
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaIPPComponentReturnOutput failed for IPP %d", __func__, ippNum);
        *ctx->quit = NVMEDIA_TRUE;
        return status;
    }


#ifdef MULTI_EGL_STREAM
	eglBuffer->image->tag += 2;
    LOG_ERR("%p %p %d\n", eglStrmProducerCtx->eglProducer[ippNum], eglBuffer->image, ippNum);
    status = NvMediaEglStreamProducerPostImage(eglStrmProducerCtx->eglProducer[ippNum],
                                                  eglBuffer->image,
                                                  NULL);
    if(IsFailed(status)) {
        LOG_ERR("%s: NvMediaEglStreamProducerPostImage failed, %d\n", __func__, status);
        return  status;
    }
    if(IsFailed(NvMediaEglStreamProducerPostImage(eglStrmProducerCtx->eglProducer[ippNum+4],
                                                  eglBuffer->image,
                                                  NULL))) {
        LOG_ERR("%s: NvMediaEglStreamProducerPostImage failed\n", __func__);
        return  status;
    }
    // The first ProducerGetImage() has to happen
    // after the second ProducerPostImage()
    if(!ctx->eglProducerGetImageFlag[ippNum]) {
        ctx->eglProducerGetImageFlag[ippNum] = NVMEDIA_TRUE;
        return status;
    }

    status = NvMediaEglStreamProducerGetImage(eglStrmProducerCtx->eglProducer[ippNum],
                                                  &retImage,
                                                  timeoutMS);

    if(status == NVMEDIA_STATUS_OK) {
        LOG_DBG("%s: EGL producer # %d: Got image %p %d %d\n", __func__, ippNum, retImage, retImage->height, retImage->width);
        retImage->tag--;
        if (((uint64_t)retImage->tag & 0x01) == 0) {       
            LOG_ERR("%s: EGL producer # %d: Got image %p and return\n", __func__, ippNum, retImage);
            retBuffer = (ImageBuffer *)retImage->tag;
            BufferPool_ReleaseBuffer(retBuffer->bufferPool, retBuffer);
        }
    }
    else {
        LOG_DBG("NvMediaEglStreamProducerGetImage waiting\n");
    }
    status = NvMediaEglStreamProducerGetImage(eglStrmProducerCtx->eglProducer[ippNum+4],
                                                  &retImage,
                                                  timeoutMS);

    if(status == NVMEDIA_STATUS_OK) {
        LOG_DBG("%s: EGL producer # %d: Got image %p\n", __func__, ippNum, retImage);
        retImage->tag--;
        if (((uint64_t)retImage->tag & 0x01) == 0) {       
            LOG_ERR("%s: EGL producer # %d: Got image %p and return\n", __func__, ippNum, retImage);
            retBuffer = (ImageBuffer *)retImage->tag;
            BufferPool_ReleaseBuffer(retBuffer->bufferPool, retBuffer);
        }
    }
    else {
        LOG_DBG("NvMediaEglStreamProducerGetImage waiting\n");
    }
#else

    LOG_ERR("%s: EGL producer: Post image %p %d\n", __func__, output->image, ippNum);
    if(IsFailed(NvMediaEglStreamProducerPostImage(eglStrmProducerCtx->eglProducer[ippNum],
                                                  output->image,
                                                  NULL))) {
        LOG_ERR("%s: NvMediaEglStreamProducerPostImage failed\n", __func__);
        return  status;
    }

    // The first ProducerGetImage() has to happen
    // after the second ProducerPostImage()
    if(!ctx->eglProducerGetImageFlag[ippNum]) {
        ctx->eglProducerGetImageFlag[ippNum] = NVMEDIA_TRUE;
        return status;
    }

    // get image from eglstream and release it
    do {
        status = NvMediaEglStreamProducerGetImage(eglStrmProducerCtx->eglProducer[ippNum],
                                                  &retImage,
                                                  timeoutMS);
        retry--;
    } while(retry >= 0 && !retImage && !(*(ctx->quit)));

    if(retImage && status == NVMEDIA_STATUS_OK) {
        LOG_DBG("%s: EGL producer # %d: Got image %p\n", __func__, ippNum, retImage);
        retOutput.image = retImage;
        // Return processed image to IPP
        status = NvMediaIPPComponentReturnOutput(ctx->outputComponent[ippNum], //component
                                                 &retOutput);                //output image
        if (status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s: NvMediaIPPComponentReturnOutput failed %d", __func__, ippNum);
            *ctx->quit = NVMEDIA_TRUE;
            return status;
        }
    }
    else {
        LOG_DBG("%s: EGL producer: no return image\n", __func__);
        *ctx->quit = NVMEDIA_TRUE;
        status = NVMEDIA_STATUS_ERROR;
    }
#endif
    return status;
}

void
ImageProducerProc (
    void *data,
    void *user_data)
{
    ImageProducerCtx *ctx = (ImageProducerCtx *)data;
    NvMediaStatus status;
    NvU32 i;
    if(!ctx) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return;
    }

    LOG_ERR("Get IPP output thread is active, ippNum=%d\n", ctx->ippNum);
    while(!(*ctx->quit)) {
        for(i = 0; i < ctx->ippNum; i++) {
            NvMediaIPPComponentOutput output;

            // Get images from ipps
            status = NvMediaIPPComponentGetOutput(ctx->outputComponent[i], //component
                                                  GET_FRAME_TIMEOUT,       //millisecondTimeout,
                                                  &output);                //output image

            if (status == NVMEDIA_STATUS_OK) {
                if(ctx->showTimeStamp) {
                    NvMediaGlobalTime globalTimeStamp;

                    if(IsSucceed(NvMediaImageGetGlobalTimeStamp(output.image, &globalTimeStamp))) {
                        LOG_INFO("IPP: Pipeline: %d Timestamp: %lld.%06lld\n", i,
                            globalTimeStamp / 1000000, globalTimeStamp % 1000000);
                    } else {
                        LOG_ERR("%s: Get time-stamp failed\n", __func__);
                        *ctx->quit = NVMEDIA_TRUE;
                    }
                }

                if(ctx->showMetadataFlag) {
                    PrintMetadataInfo(ctx->outputComponent[i], &output);
                }
                status = SendIPPHumanVisionOutToEglStream(ctx,i,&output);

                if(status != NVMEDIA_STATUS_OK) {
                    *ctx->quit = NVMEDIA_TRUE;
                    break;
                }
            }
        } // for loop
    } // while loop

    *ctx->producerExited = NVMEDIA_TRUE;
}

static NvMediaStatus
Set2DProcessingParams (
    ImageProducerCtx *ctx,
    NvMediaBool aggregateFlag)
{
    NvMedia2DBlitParameters *params = ctx->blitParams;
    NvU32 width;

    params->validFields = 0;
    params->flags = 0;
    params->filter = 1;
    params->dstTransform = 0;
    params->validFields = 0;
    params->colorStandard = 0;

    if(aggregateFlag)
        width = ctx->width/ctx->ippNum;
    else
        width = ctx->width;

    // Default values in the config file for full resolution
    if(ctx->srcRect.x0 == 0 &&
       ctx->srcRect.y0 == 0 &&
       ctx->srcRect.x1 == 0 &&
       ctx->srcRect.y1 == 0) {
        SetRect(&ctx->srcRect, 0, 0, width, ctx->height);
    }

    // Default values in the config file for full resolution
    if(ctx->dstRect.x0 == 0 &&
       ctx->dstRect.y0 == 0 &&
       ctx->dstRect.x1 == 0 &&
       ctx->dstRect.y1 == 0) {
        SetRect(&ctx->dstRect, 0, 0, width, ctx->height);
    }

    return NVMEDIA_STATUS_OK;
}

ImageProducerCtx*
ImageProducerInit(NvMediaDevice *device,
                  EglStreamClient *streamClient,
                  NvU32 width, NvU32 height,
                  InteropContext *interopCtx)
{
    NvU32 i,j;
    ImageBufferPoolConfigNew poolConfig;
    ImageProducerCtx *client = NULL;

    if(!device) {
        LOG_ERR("%s: invalid NvMedia device\n", __func__);
        return NULL;
    }

    client = malloc(sizeof(ImageProducerCtx));
    if (!client) {
        LOG_ERR("%s:: failed to alloc memory\n", __func__);
        return NULL;
    }
    memset(client, 0, sizeof(ImageProducerCtx));

    client->device = device;
    client->width = width;
    client->height = height;
    client->ippNum = interopCtx->ippNum;
    client->surfaceType = interopCtx->eglProdSurfaceType;
    client->eglDisplay = streamClient->display;
    client->producerExited = &interopCtx->producerExited;
    client->quit = interopCtx->quit;
    client->showTimeStamp = interopCtx->showTimeStamp;
    client->showMetadataFlag = interopCtx->showMetadataFlag;
    // Create 2D for blit, surface format conversion
    {
        client->i2d = NvMedia2DCreate(interopCtx->device);
        if(!client->i2d) {
            LOG_ERR("%s: Failed to create NvMedia 2D i2d\n", __func__);
            goto fail;
        }

        client->blitParams = calloc(1, sizeof(NvMedia2DBlitParameters));
        if(!client->blitParams) {
            LOG_ERR("%s: Out of memory", __func__);
            goto fail;
        }

        if(IsFailed(Set2DProcessingParams(client, 0))) {//interopCtx->aggregateFlag))) {
            goto fail;
        }
    }

    // Create 2D/display buffers, queues and threads
    if(IsFailed(NvMutexCreate(&client->processorMutex))) {
        goto fail;
    }
    NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);
    //NVM_SURF_FMT_SET_ATTR_YUV(surfFormatAttrs,YUV,420,SEMI_PLANAR,UINT,8,PL);
    NVM_SURF_FMT_SET_ATTR_RGBA(surfFormatAttrs,RGBA,UINT,8,PL);
    memset(&poolConfig, 0, sizeof(ImageBufferPoolConfigNew));
    poolConfig.surfType = NvMediaSurfaceFormatGetType(surfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);
    poolConfig.device = interopCtx->device;
    poolConfig.surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
    //poolConfig.surfAllocAttrs[0].value = interopCtx->width / interopCtx->ippNum;
    poolConfig.surfAllocAttrs[0].value = interopCtx->width;
    poolConfig.surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
    poolConfig.surfAllocAttrs[1].value = interopCtx->height;
    poolConfig.numSurfAllocAttrs = 2;
    for(i=0; i < interopCtx->ippNum; i++) {
        if(client->bufferPool[i]) {
            BufferPool_Destroy(client->bufferPool[i]);
        }

        if(IsFailed(BufferPool_Create_New(&client->bufferPool[i],// Buffer pool
                                  IMAGE_BUFFERS_POOL_SIZE,  // Capacity
                                  BUFFER_POOL_TIMEOUT,      // Timeout
                                  IMAGE_BUFFER_POOL,        // Pool type
                                  &poolConfig))) {          // Config
            LOG_ERR("%s: BufferPool_Create_New failed", __func__);
            goto fail;
        }

        LOG_DBG("%s: BufferPool_Create_New: eglStep.bufferPool: %ux%u\n",
                __func__, (interopCtx->width / interopCtx->ippNum), interopCtx->height);
    }
#ifdef MULTI_EGL_STREAM
    for(j=0; j< interopCtx->ippNum; j++) {
        client->outputComponent[j] = interopCtx->outputComponent[j];
        for(i=j; i<j+8; i+=4) {
            // Create EGL stream producer
            EGLint streamState = 0;
            client->eglStream[i]   = streamClient->eglStream[i];
            while(streamState != EGL_STREAM_STATE_CONNECTING_KHR) {
                if(!eglQueryStreamKHRfp(streamClient->display,
                            streamClient->eglStream[i],
                            EGL_STREAM_STATE_KHR,
                            &streamState)) {
                    LOG_ERR("eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
                }
            }

            LOG_ERR("%d %d 0x%x\n", client->width, client->height, client->surfaceType);
            client->eglProducer[i] = NvMediaEglStreamProducerCreate(client->device,
                    client->eglDisplay,
                    client->eglStream[i],
                    client->surfaceType,
                    client->width,
                    client->height);
            if(!client->eglProducer[i]) {
                LOG_ERR("%s: Failed to create EGL producer\n", __func__);
                goto fail;
            }
            //set multiSend according to NV's document
            NvMediaEglStreamProducerAttributes producerAttributes = { .multiSend = NVMEDIA_TRUE};
            NvMediaEglStreamProducerSetAttributes(client->eglProducer[i], NVMEDIA_EGL_STREAM_PRODUCER_ATTRIBUTE_MULTISEND, &producerAttributes);
        }
    }
#else
    for(i=0; i< interopCtx->ippNum; i++) {
        client->outputComponent[i] = interopCtx->outputComponent[i];
        // Create EGL stream producer
        EGLint streamState = 0;
        client->eglStream[i]   = streamClient->eglStream[i];
        while(streamState != EGL_STREAM_STATE_CONNECTING_KHR) {
           if(!eglQueryStreamKHRfp(streamClient->display,
                                 streamClient->eglStream[i],
                                 EGL_STREAM_STATE_KHR,
                                 &streamState)) {
               LOG_ERR("eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
            }
        }

        LOG_ERR("%d %d %x\n", client->width, client->height, client->surfaceType);
        client->eglProducer[i] = NvMediaEglStreamProducerCreate(client->device,
                                                                client->eglDisplay,
                                                                client->eglStream[i],
                                                                client->surfaceType,
                                                                client->width,
                                                                client->height);
        if(!client->eglProducer[i]) {
            LOG_ERR("%s: Failed to create EGL producer\n", __func__);
            goto fail;
        }
    }
#endif
    return client;
fail:
    ImageProducerFini(client);
    return NULL;
}
NvMediaStatus ImageProducerFini(ImageProducerCtx *ctx) {
    NvU32 i;
    NvMediaImage *retImage = NULL;
    ImageBuffer *retBuffer = NULL;
    LOG_DBG("ImageProducerFini: start\n");

    if(ctx != NULL) {

        if(ctx->processorMutex != NULL) {
            NvMutexDestroy(ctx->processorMutex);
        }

        for(i = 0; i < ctx->ippNum; i++) {
            // Finalize
            do {
                retImage = NULL;
                NvMediaEglStreamProducerGetImage(ctx->eglProducer[i],
                                                 &retImage,
                                                 0);
                if(retImage) {
                    LOG_DBG("%s: EGL producer: Got image %p\n", __func__, retImage);
                    retBuffer = (ImageBuffer *)retImage->tag;
                    BufferPool_ReleaseBuffer(retBuffer->bufferPool, retBuffer);
                }
            } while(retImage);
        }

        for(i=0; i < ctx->ippNum; i++) {
            BufferPool_Destroy(ctx->bufferPool[i]);
        }

        for(i=0; i<ctx->ippNum; i++) {
            if(ctx->eglProducer[i])
                NvMediaEglStreamProducerDestroy(ctx->eglProducer[i]);
        }

        if(ctx->i2d)
            NvMedia2DDestroy(ctx->i2d);
        if(ctx->blitParams)
            free(ctx->blitParams);

        free(ctx);
    }

    LOG_DBG("ImageProducerFini: end\n");
    return NVMEDIA_STATUS_OK;
}

