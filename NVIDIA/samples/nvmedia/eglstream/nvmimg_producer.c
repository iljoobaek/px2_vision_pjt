/*
 * eglimageproducer.c
 *
 * Copyright (c) 2015-2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

//
// DESCRIPTION:   Simple EGL stream producer app for Image2D
//

#include "nvmimg_producer.h"

#define FRAMES_PER_SECOND      30

extern NvBool signal_stop;
Image2DTestArgs g_testArgs;

// Set2DParams() is a helper function for setting
// the 2D processing parameters in the Image2DTestArgs
// structure with default settings
static NvMediaStatus
Set2DParams (Image2DTestArgs *args)
{
    NvMedia2DBlitParameters *blitParams = args->blitParams;

    //set default blit params
    blitParams->validFields = 0;
    blitParams->flags = 0;
    blitParams->filter = 1;     //NVMEDIA_2D_STRETCH_FILTER_OFF (Disable the horizontal and vertical filtering)
    blitParams->validFields |= blitParams->filter > 1 ?
                                NVMEDIA_2D_BLIT_PARAMS_FILTER :
                                0;
    blitParams->dstTransform = 0; //NVMEDIA_2D_TRANSFORM_FLIP_VERTICAL
    blitParams->validFields |= blitParams->dstTransform > 0 ?
                                NVMEDIA_2D_BLIT_PARAMS_DST_TRANSFORM :
                                0;
    blitParams->colorStandard = 0; //NVMEDIA_COLOR_STANDARD_ITUR_BT_601

    SetRect(args->srcRect, 0, 0, args->inputWidth, args->inputHeight);

    SetRect(args->dstRect, 0, 0, args->outputWidth, args->outputHeight);

    return NVMEDIA_STATUS_OK;
}

static NvU32
GetFramesCount(Image2DTestArgs *ctx)
{
    NvU32 count = 0, minimalFramesCount = 0, imageSize = 0;
    FILE **inputFile = NULL;
    NVM_SURF_FMT_DEFINE_ATTR(attr);
    NvMediaStatus status;

    inputFile = calloc(1, sizeof(FILE *));
    if(!inputFile) {
        LOG_ERR("%s: Out of memory", __func__);
        return 0;
    }

    status = NvMediaSurfaceFormatGetAttrs(ctx->inputSurfType,
                                          attr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        goto done;
    }

    // Get input file frames count
    if(strstr(ctx->inputImages, ".rgba")) {
        imageSize = ctx->inputWidth * ctx->inputHeight *4;
    } else {
        if(strstr(ctx->inputImages, ".rgba")) {
            imageSize = ctx->inputWidth * ctx->inputHeight *4;
        } else {
            if((attr[NVM_SURF_ATTR_COMPONENT_ORDER].value == NVM_SURF_ATTR_COMPONENT_ORDER_YUV) &&
               (attr[NVM_SURF_ATTR_SUB_SAMPLING_TYPE].value == NVM_SURF_ATTR_SUB_SAMPLING_TYPE_420)) {
                imageSize =  ctx->inputWidth * ctx->inputHeight * 3 / 2;
            } else {
                LOG_ERR("%s: Invalid image surface type %u\n", __func__, ctx->inputSurfType);
                goto done;
            }
        }
        if(imageSize == 0) {
            LOG_ERR("%s: Bad image parameters", __func__);
            minimalFramesCount = 0;
            goto done;
        }
    }
    if(imageSize == 0) {
        LOG_ERR("%s: Bad image parameters", __func__);
        minimalFramesCount = 0;
        goto done;
    }

    if(!(inputFile[0] = fopen(ctx->inputImages, "rb"))) {
        LOG_ERR("%s: Failed opening file %s", __func__,
                ctx->inputImages);
        minimalFramesCount = 0;
        goto done;
    }

    fseek(inputFile[0], 0, SEEK_END);
    count = ftell(inputFile[0]) / imageSize;
    minimalFramesCount = count;

    fclose(inputFile[0]);

done:
    if (inputFile) {
        free(inputFile);
        inputFile = NULL;
    }
    return minimalFramesCount;
}

static NvU32
ProcessImageThread(void * args)
{
    Image2DTestArgs *testArgs = (Image2DTestArgs *)args;
    ImageBuffer *inputImageBuffer = NULL;
    ImageBuffer *outputImageBuffer = NULL;
    ImageBuffer *releaseBuffer = NULL;
    NvMediaImage *releaseImage = NULL;
    NvMediaStatus status;
    NvU32 framesCount = 1;
    NvU32 readFrame = 0;
    char *inputFileName = NULL;
    NvMediaTime timeStamp ={0, 0};
    NVM_SURF_FMT_DEFINE_ATTR(attr);
    framesCount = GetFramesCount(testArgs);
    if(framesCount == 0) {
        LOG_ERR("%s: GetFramesCount() failed", __func__);
        goto done;
    }

    inputFileName = testArgs->inputImages;

    while(!signal_stop) {
        // Acquire Image from inputBufferpool
        if(IsFailed(BufferPool_AcquireBuffer(testArgs->inputBuffersPool,
                                             (void *)&inputImageBuffer))){
            LOG_DBG("%s: Input BufferPool_AcquireBuffer waiting \n", __func__);
            goto getImage;
        }

        status = NvMediaSurfaceFormatGetAttrs(testArgs->outputSurfType,
                                              attr,
                                              NVM_SURF_FMT_ATTR_MAX);
        if (status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
            goto done;
        }

        // Acquire Image from outputBufferpool
        if(attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA &&
           IsFailed(BufferPool_AcquireBuffer(testArgs->outputBuffersPool,
                                            (void *)&outputImageBuffer))){
            BufferPool_ReleaseBuffer(inputImageBuffer->bufferPool,
                                     inputImageBuffer);
            LOG_DBG("%s: Output BufferPool_AcquireBuffer waiting \n",__func__);
            goto getImage;
        }

        if(readFrame >= framesCount && testArgs->loop) {
           readFrame = 0;
           testArgs->loop--;
           if (testArgs->loop == 0)
               goto done;
        }
        //Read Image into inputImageBuffer
        LOG_DBG("%s: Reading image %u/%u\n", __func__, readFrame + 1, framesCount);
        if(IsFailed(ReadImage(inputFileName,
                              readFrame,
                              testArgs->inputWidth,
                              testArgs->inputHeight,
                              inputImageBuffer->image,
                              NVMEDIA_TRUE,                 //inputUVOrderFlag,
                              1))) {                        //rawInputBytesPerPixel
            LOG_ERR(": Failed reading frame %u", readFrame);
            BufferPool_ReleaseBuffer(inputImageBuffer->bufferPool,
                                     inputImageBuffer);
            if(attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
               BufferPool_ReleaseBuffer(outputImageBuffer->bufferPool, outputImageBuffer);
            }
            goto done;
        }
        readFrame++;
        if (testArgs->frameCount && readFrame > testArgs->frameCount)
            goto done;

        if (attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
            //Blit operation (YUV-RGB conversion,2D_TRANSFORM_FLIP_VERTICAL)
            if(IsFailed(NvMedia2DBlitEx(testArgs->blitter,
                                        outputImageBuffer->image,
                                        testArgs->dstRect,
                                        inputImageBuffer->image,
                                        testArgs->srcRect,
                                        testArgs->blitParams,
                                        NULL))) {
                LOG_ERR("ProcessImageThread: NvMedia2DBlitEx failed\n");
                BufferPool_ReleaseBuffer(inputImageBuffer->bufferPool, inputImageBuffer);
                BufferPool_ReleaseBuffer(outputImageBuffer->bufferPool, outputImageBuffer);
                goto done;
            }
        }

#ifdef EGL_NV_stream_metadata
        //Test for metaData
        if(testArgs->metadataEnable) {
            unsigned char buf[256] = {0};
            static unsigned char frameId = 0;
            int blockIdx = 0;

            memset(buf, 16, 256);
            buf[0] = frameId;
            frameId ++;
            if (frameId == 255)
                frameId =0;

            for(blockIdx=0; blockIdx<4; blockIdx++) {
                buf[1] = blockIdx;
                status = NvMediaEglStreamProducerPostMetaData(
                               testArgs->producer,
                               blockIdx,            //blockIdx
                               (void *)buf,         //dataBuf
                               blockIdx*16,         //offset
                               256);                //size
                if(status != NVMEDIA_STATUS_OK) {
                    LOG_ERR("%s: NvMediaEglStreamProducerPostMetaData failed, blockIdx %d\n", __func__, blockIdx);
                    goto done;
                }
            }
        }
#endif //EGL_NV_stream_metadata

        if (!testArgs->lDispCounter) {
            // Start timing at the first frame
            GetTimeUtil(&testArgs->baseTime);
        }

        NvAddTime(&testArgs->baseTime, (NvU64)((NvF64)(testArgs->lDispCounter + 5) * testArgs->frameTimeUSec), &timeStamp);

        // Post outputImage to egl-stream
        status = NvMediaEglStreamProducerPostImage(testArgs->producer,
                           attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA?
                           outputImageBuffer->image:inputImageBuffer->image,
                           &timeStamp);

        if (attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
            //Release the inputImageBuffer back to the bufferpool
            BufferPool_ReleaseBuffer(inputImageBuffer->bufferPool, inputImageBuffer);
        }

        if(status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s: NvMediaEglStreamProducerPostImage failed\n", __func__);
            goto done;
        }

        testArgs->lDispCounter++;

getImage:
        // Get back from the egl-stream
        status = NvMediaEglStreamProducerGetImage(testArgs->producer,
                                                  &releaseImage,
                                                  0);
        if (status == NVMEDIA_STATUS_OK) {
            //Return the image back to the bufferpool
            releaseBuffer = (ImageBuffer *)releaseImage->tag;
            BufferPool_ReleaseBuffer(releaseBuffer->bufferPool,
                                     releaseBuffer);
         } else {
            LOG_DBG ("%s: NvMediaEglStreamProducerGetImage waiting\n", __func__);
            continue;
         }

    }

done:
    *testArgs->producerStop = NV_TRUE;

    return 0;
}

static NvMediaStatus
Image2D_Init (Image2DTestArgs *testArgs, NvMediaSurfaceType outputSurfaceType)
{
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;
    ImageBufferPoolConfigNew imagesPoolConfig;
    NVM_SURF_FMT_DEFINE_ATTR(attr);
    char *env;

    if(!testArgs) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }

    testArgs->device = NvMediaDeviceCreate();
    if(!testArgs->device) {
        LOG_ERR("%s: Failed to create NvMedia device\n", __func__);
        goto failed;
    }

    testArgs->blitter = NvMedia2DCreate(testArgs->device);
    if(!testArgs->blitter) {
        LOG_ERR("%s: Failed to create NvMedia 2D blitter\n", __func__);
        goto failed;
    }

    testArgs->blitParams = calloc(1, sizeof(NvMedia2DBlitParameters));
    if(!testArgs->blitParams) {
        LOG_ERR("%s: Out of memory", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto failed;
    }

    testArgs->srcRect = calloc(1, sizeof(NvMediaRect));
    testArgs->dstRect = calloc(1, sizeof(NvMediaRect));
    if(!testArgs->srcRect || !testArgs->dstRect) {
        LOG_ERR("%s: Out of memory", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto failed;
    }

    if (!testArgs->pitchLinearOutput) {
        LOG_DBG("Image producer block linear used\n");
    } else {
        LOG_DBG("Image producer Pitch linear used\n");
    }

    status = NvMediaSurfaceFormatGetAttrs(testArgs->inputSurfType,
                                          attr,
                                          NVM_SURF_FMT_ATTR_MAX);

    memset(&imagesPoolConfig, 0, sizeof(ImageBufferPoolConfigNew));
    imagesPoolConfig.surfType = testArgs->inputSurfType;
    imagesPoolConfig.device = testArgs->device;
    imagesPoolConfig.surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
    imagesPoolConfig.surfAllocAttrs[0].value = testArgs->inputWidth;
    imagesPoolConfig.surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
    imagesPoolConfig.surfAllocAttrs[1].value = testArgs->inputHeight;
    imagesPoolConfig.surfAllocAttrs[2].type = NVM_SURF_ATTR_CPU_ACCESS;
    imagesPoolConfig.surfAllocAttrs[2].value = NVM_SURF_ATTR_CPU_ACCESS_UNMAPPED;
    imagesPoolConfig.numSurfAllocAttrs = 3;
    if(testArgs->directFlip) {
        imagesPoolConfig.surfAllocAttrs[3].type = NVM_SURF_ATTR_ALLOC_TYPE;
        imagesPoolConfig.surfAllocAttrs[3].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
        imagesPoolConfig.numSurfAllocAttrs += 1;
        if ((env = getenv("DISPLAY_VM"))) {
            imagesPoolConfig.surfAllocAttrs[4].type = NVM_SURF_ATTR_PEER_VM_ID;
            imagesPoolConfig.surfAllocAttrs[4].value = atoi(env);
            imagesPoolConfig.numSurfAllocAttrs += 1;
        }
    }

#ifndef NVMEDIA_GHSI
    else if((testArgs->crossPartProducer == NV_TRUE)) {
        imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].type = NVM_SURF_ATTR_ALLOC_TYPE;
        imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
        imagesPoolConfig.numSurfAllocAttrs += 1;
        imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].type = NVM_SURF_ATTR_PEER_VM_ID;
        imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].value = testArgs->consumerVmId;
        imagesPoolConfig.numSurfAllocAttrs += 1;
    }
#endif

    if(IsFailed(BufferPool_Create_New(&testArgs->inputBuffersPool, // Buffer pool
                                  IMAGE_BUFFERS_POOL_SIZE,     // Capacity
                                  BUFFER_POOL_TIMEOUT,         // Timeout
                                  IMAGE_BUFFER_POOL,           // Buffer pool type
                                  &imagesPoolConfig))) {       // Config
        LOG_ERR("Image2D_Init: BufferPool_Create_New failed");
        goto failed;
    }

    status = NvMediaSurfaceFormatGetAttrs(outputSurfaceType,
                                          attr,
                                          NVM_SURF_FMT_ATTR_MAX);

    memset(&imagesPoolConfig, 0, sizeof(ImageBufferPoolConfigNew));
    imagesPoolConfig.surfType = outputSurfaceType;
    imagesPoolConfig.surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
    imagesPoolConfig.surfAllocAttrs[0].value = testArgs->outputWidth;
    imagesPoolConfig.surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
    imagesPoolConfig.surfAllocAttrs[1].value = testArgs->outputHeight;
    imagesPoolConfig.surfAllocAttrs[2].type = NVM_SURF_ATTR_CPU_ACCESS;
    imagesPoolConfig.surfAllocAttrs[2].value = NVM_SURF_ATTR_CPU_ACCESS_UNMAPPED;
    imagesPoolConfig.numSurfAllocAttrs = 3;
    imagesPoolConfig.device = testArgs->device;
    if(testArgs->directFlip) {
        imagesPoolConfig.surfAllocAttrs[3].type = NVM_SURF_ATTR_ALLOC_TYPE;
        imagesPoolConfig.surfAllocAttrs[3].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
        imagesPoolConfig.numSurfAllocAttrs += 1;
        if ((env = getenv("DISPLAY_VM"))) {
            imagesPoolConfig.surfAllocAttrs[4].type = NVM_SURF_ATTR_PEER_VM_ID;
            imagesPoolConfig.surfAllocAttrs[4].value = atoi(env);
            imagesPoolConfig.numSurfAllocAttrs += 1;
        }
    }

#ifndef NVMEDIA_GHSI
    else if((testArgs->crossPartProducer == NV_TRUE)) {
        imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].type = NVM_SURF_ATTR_ALLOC_TYPE;
        imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
        imagesPoolConfig.numSurfAllocAttrs += 1;
        imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].type = NVM_SURF_ATTR_PEER_VM_ID;
        imagesPoolConfig.surfAllocAttrs[imagesPoolConfig.numSurfAllocAttrs].value = testArgs->consumerVmId;
        imagesPoolConfig.numSurfAllocAttrs += 1;
    }
#endif

    if(IsFailed(BufferPool_Create_New(&testArgs->outputBuffersPool, // Buffer pool
                                  IMAGE_BUFFERS_POOL_SIZE,      // Capacity
                                  BUFFER_POOL_TIMEOUT,          // Timeout
                                  IMAGE_BUFFER_POOL,            // Buffer pool type
                                  &imagesPoolConfig))) {        // Config
        LOG_ERR("Image2D_Init: BufferPool_Create_New failed");
        goto failed;
    }

    if(IsFailed(Set2DParams(testArgs)))
        goto failed;

    return NVMEDIA_STATUS_OK;

failed:
    LOG_ERR("%s: Failed", __func__);
    Image2DDeinit();
    return status;
}

int
Image2DInit(volatile NvBool *imageProducerStop,
            EGLDisplay eglDisplay,
            EGLStreamKHR eglStream,
            TestArgs *args)
{
    Image2DTestArgs *testArgs = &g_testArgs;
    NvMediaStatus status;
    memset(testArgs, 0, sizeof(Image2DTestArgs));
    testArgs->inputImages = args->infile;
    testArgs->inputWidth = args->inputWidth;
    testArgs->inputHeight = args->inputHeight;
    testArgs->inputSurfType = args->prodSurfaceType;

    NVM_SURF_FMT_DEFINE_ATTR(attr);
    status = NvMediaSurfaceFormatGetAttrs(args->prodSurfaceType,
                                          attr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        *testArgs->producerStop = 1;
        return 0;
    }

    if (!((attr[NVM_SURF_ATTR_COMPONENT_ORDER].value == NVM_SURF_ATTR_COMPONENT_ORDER_YUV) &&
       (attr[NVM_SURF_ATTR_SUB_SAMPLING_TYPE].value == NVM_SURF_ATTR_SUB_SAMPLING_TYPE_420)) &&
       !((attr[NVM_SURF_ATTR_COMPONENT_ORDER].value == NVM_SURF_ATTR_COMPONENT_ORDER_LUMA) &&
       (attr[NVM_SURF_ATTR_BITS_PER_COMPONENT].value == NVM_SURF_ATTR_BITS_PER_COMPONENT_10))) {
       NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);
       NVM_SURF_FMT_SET_ATTR_YUV(surfFormatAttrs,YUV,420,SEMI_PLANAR,UINT,8,PL);
       if (!args->pitchLinearOutput){
           surfFormatAttrs[NVM_SURF_ATTR_LAYOUT].value = NVM_SURF_ATTR_LAYOUT_BL;
       }
       testArgs->inputSurfType = NvMediaSurfaceFormatGetType(surfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);
    }

    //output dimension same as input dimension
    testArgs->outputWidth = args->inputWidth;
    testArgs->outputHeight = args->inputHeight;
    testArgs->outputSurfType = args->prodSurfaceType;
    testArgs->pitchLinearOutput = args->pitchLinearOutput;
    testArgs->loop              = args->prodLoop? args->prodLoop : 1;
    testArgs->frameCount        = args->prodFrameCount;
    testArgs->directFlip        = (args->egloutputConsumer
#ifdef EGLOUTPUT_SUPPORT
                                  || (args->crossConsumerType == EGLOUTPUT_CONSUMER)
#endif
                                  ) ? 1 : 0;
    if (args->prodLoop)
        testArgs->frameCount = 0;  //Only use loop count if it is valid
    testArgs->lDispCounter = 0;
    testArgs->frameTimeUSec = (1000000.0/FRAMES_PER_SECOND);
    testArgs->eglDisplay = eglDisplay;
    testArgs->eglStream = eglStream;
    testArgs->producerStop = imageProducerStop;
    testArgs->metadataEnable = args->metadata;

    testArgs->crossPartProducer = (args->standalone == STANDALONE_CP_PRODUCER) ? 1 : 0;
    if (testArgs->crossPartProducer == NV_TRUE) {
        testArgs->consumerVmId = args->consumerVmId;
    }

    if(IsFailed(Image2D_Init(testArgs, args->prodSurfaceType))) {
        *testArgs->producerStop = 1;
        return 0;
    }

    // Create EGLStream-Producer
    testArgs->producer = NvMediaEglStreamProducerCreate(testArgs->device,
                                                        testArgs->eglDisplay,
                                                        testArgs->eglStream,
                                                        args->prodSurfaceType,
                                                        testArgs->outputWidth,
                                                        testArgs->outputHeight);
    if(!testArgs->producer) {
        LOG_ERR("Image2DInit: Unable to create producer\n");
        Image2DDeinit();
        return 0;
    }

    if (IsFailed(NvThreadCreate(&testArgs->thread, &ProcessImageThread, (void*)testArgs, NV_THREAD_PRIORITY_NORMAL))) {
        LOG_ERR("Image2DInit: Unable to create ProcessImageThread\n");
        return 0;
    }

    return 1;
}


void Image2DDeinit()
{
    Image2DTestArgs *testArgs = &g_testArgs;

    if(testArgs->inputBuffersPool)
        BufferPool_Destroy(testArgs->inputBuffersPool);

    if(testArgs->outputBuffersPool)
        BufferPool_Destroy(testArgs->outputBuffersPool);

    if(testArgs->blitter)
        NvMedia2DDestroy(testArgs->blitter);

    if(testArgs->dstRect) {
        free(testArgs->dstRect);
        testArgs->dstRect = NULL;
    }

    if(testArgs->srcRect) {
        free(testArgs->srcRect);
        testArgs->srcRect = NULL;
    }

    if(testArgs->blitParams) {
        free(testArgs->blitParams);
        testArgs->blitParams = NULL;
    }

    if(testArgs->producer) {
        NvMediaEglStreamProducerDestroy(testArgs->producer);
        testArgs->producer = NULL;
    }

    if (testArgs->device) {
        NvMediaDeviceDestroy(testArgs->device);
        testArgs->device = NULL;
    }
}

void Image2DproducerStop() {
    Image2DTestArgs *testArgs = &g_testArgs;

    LOG_DBG("Image2DDeinit, wait for thread stop\n");
    if(testArgs->thread) {
        NvThreadDestroy(testArgs->thread);
    }
    LOG_DBG("Image2DDeinit, thread stop\n");
}

void Image2DproducerFlush() {
    Image2DTestArgs *testArgs = &g_testArgs;
    NvMediaImage *releaseImage = NULL;
    ImageBuffer *releaseBuffer = NULL;

    while (NvMediaEglStreamProducerGetImage(
           testArgs->producer, &releaseImage, 0) == NVMEDIA_STATUS_OK) {
        releaseBuffer = (ImageBuffer *)releaseImage->tag;
        BufferPool_ReleaseBuffer(releaseBuffer->bufferPool,
                                 releaseBuffer);
    }
}
