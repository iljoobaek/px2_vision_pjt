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

// RAW Pipeline
static NvMediaStatus
SendIPPHumanVisionOutToEglStream(
    ImageProducerCtx *ctx,
    NvU32 ippNum,
    NvMediaIPPComponentOutput *output)
{
    NvMediaImage *retImage = NULL;
    NvMediaStatus status = NVMEDIA_STATUS_OK;
    NvU32 timeoutMS = EGL_PRODUCER_TIMEOUT_MS * ctx->ippNum;
    int retry = EGL_PRODUCER_GET_IMAGE_MAX_RETRIES;
    NvMediaIPPComponentOutput retOutput;
    ImageProducerCtx*   eglStrmProducerCtx;

    eglStrmProducerCtx = ctx;

#ifdef MULTI_EGL_STREAM
	output->image->tag = 2;
    LOG_ERR("%p %p %d\n", eglStrmProducerCtx->eglProducer[ippNum], output->image, ippNum);
    status = NvMediaEglStreamProducerPostImage(eglStrmProducerCtx->eglProducer[ippNum],
                                                  output->image,
                                                  NULL);
    //if(IsFailed(NvMediaEglStreamProducerPostImage(eglStrmProducerCtx->eglProducer[ippNum],
    //                                              output->image,
    //                                              NULL))) {
    if(IsFailed(status)) {
        LOG_ERR("%s: NvMediaEglStreamProducerPostImage failed, %d\n", __func__, status);
        return  status;
    }
    if(IsFailed(NvMediaEglStreamProducerPostImage(eglStrmProducerCtx->eglProducer[ippNum+4],
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

    status = NvMediaEglStreamProducerGetImage(eglStrmProducerCtx->eglProducer[ippNum],
                                                  &retImage,
                                                  timeoutMS);

    if(status == NVMEDIA_STATUS_OK) {
        LOG_DBG("%s: EGL producer # %d: Got image %p %d %d\n", __func__, ippNum, retImage, retImage->height, retImage->width);
        output->image->tag--;
        if (output->image->tag == 0) {       
            LOG_ERR("%s: EGL producer # %d: Got image %p and return\n", __func__, ippNum, retImage);
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
    }
    else {
        LOG_DBG("NvMediaEglStreamProducerGetImage waiting\n");
    }
    status = NvMediaEglStreamProducerGetImage(eglStrmProducerCtx->eglProducer[ippNum+4],
                                                  &retImage,
                                                  timeoutMS);

    if(status == NVMEDIA_STATUS_OK) {
        LOG_DBG("%s: EGL producer # %d: Got image %p\n", __func__, ippNum, retImage);
        output->image->tag--;
        if (output->image->tag == 0) {
            LOG_ERR("%s: EGL producer # %d: Got image %p and return\n", __func__, ippNum, retImage);
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

ImageProducerCtx*
ImageProducerInit(NvMediaDevice *device,
                  EglStreamClient *streamClient,
                  NvU32 width, NvU32 height,
                  InteropContext *interopCtx)
{
    NvU32 i,j;
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

NvMediaStatus ImageProducerFini(ImageProducerCtx *ctx)
{
    NvU32 i;
    NvMediaImage *retImage = NULL;
    NvMediaIPPComponentOutput output;
    LOG_DBG("ImageProducerFini: start\n");
    if(ctx) {
        for(i = 0; i < ctx->ippNum; i++) {
            // Finalize
            do {
                retImage = NULL;
                NvMediaEglStreamProducerGetImage(ctx->eglProducer[i],
                                                 &retImage,
                                                 0);
                if(retImage) {
                    LOG_DBG("%s: EGL producer: Got image %p\n", __func__, retImage);
                    output.image = retImage;
                    NvMediaIPPComponentReturnOutput(ctx->outputComponent[i], //component
                                                        &output);                //output image
                }
            } while(retImage);
        }

        for(i=0; i<ctx->ippNum; i++) {
            if(ctx->eglProducer[i])
                NvMediaEglStreamProducerDestroy(ctx->eglProducer[i]);
        }
        free(ctx);
    }
    LOG_DBG("ImageProducerFini: end\n");
    return NVMEDIA_STATUS_OK;
}
