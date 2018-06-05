/* Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "capture.h"
#include "save.h"

static NvMediaStatus
_SetExtImgDevParameters(
                        NvCaptureContext *ctx,
                        CaptureConfigParams *captureParams,
                        ExtImgDevParam *configParam)
{
    configParam->desAddr = captureParams->desAddr;
    configParam->brdcstSerAddr = captureParams->brdcstSerAddr;
    configParam->brdcstSensorAddr =
        captureParams->brdcstSensorAddr;

    configParam->serAddr[CAPTURE_SENSOR_ID] = captureParams->serAddr[CAPTURE_SENSOR_ID];
    configParam->sensorAddr[CAPTURE_SENSOR_ID] = captureParams->sensorAddr[CAPTURE_SENSOR_ID];
    configParam->i2cDevice = captureParams->i2cDevice;
    configParam->moduleName = captureParams->inputDevice;
    configParam->board = captureParams->board;
    configParam->resolution = captureParams->resolution;
    configParam->camMap = &ctx->testArgs->camMap;
    configParam->sensorsNum = CAPTURE_SENSOR_NUMBER;
    configParam->inputFormat = captureParams->inputFormat;
    configParam->interface = captureParams->interface;
    configParam->enableEmbLines =
        (captureParams->embeddedDataLinesTop || captureParams->embeddedDataLinesBottom) ?
            NVMEDIA_TRUE : NVMEDIA_FALSE;
    configParam->initialized = NVMEDIA_FALSE;
    configParam->enableSimulator = NVMEDIA_FALSE;
    configParam->slave = ctx->testArgs->slaveTegra;
    configParam->enableVirtualChannels = NVMEDIA_FALSE;
    configParam->enableExtSync = ctx->testArgs->enableExtSync;
    configParam->dutyRatio =
        ((ctx->testArgs->dutyRatio <= 0.0) || (ctx->testArgs->dutyRatio >= 1.0)) ?
            0.25 : ctx->testArgs->dutyRatio;

    return NVMEDIA_STATUS_OK;
}

static NvMediaStatus
_SetICPSettings(
                CaptureThreadCtx *ctx,
                NvMediaICPSettings *icpSettings,
                ExtImgDevProperty *property,
                CaptureConfigParams *captureParams,
                TestArgs *testArgs)
{
    char *surfaceFormat = captureParams->surfaceFormat;

    NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);

    /* Set surface format */
    /* Only OV10635 camera is supported so only
     * possible surface format is yv16
     */
    if (!strcasecmp(surfaceFormat, "yv16")) {
        NVM_SURF_FMT_SET_ATTR_YUV(surfFormatAttrs,YUV,422,PLANAR,UINT,8,PL);
        ctx->rawBytesPerPixel = 0;
    } else {
        LOG_ERR("%s: Bad surface format specified: %s \n",
                __func__, surfaceFormat);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }
    ctx->surfType = NvMediaSurfaceFormatGetType(surfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);

    /* Set NvMediaICPSettings */
    icpSettings->interfaceType = property->interface;
    icpSettings->inputFormat.inputFormatType = property->inputFormatType;
    icpSettings->inputFormat.bitsPerPixel = property->bitsPerPixel;
    icpSettings->tpgEnable = property->tpgEnable;
    icpSettings->surfaceType = ctx->surfType;
    icpSettings->width = property->width;
    icpSettings->height = property->height;
    icpSettings->startX = 0;
    icpSettings->startY = 0;
    icpSettings->embeddedDataLines = property->embLinesTop + property->embLinesBottom;
    icpSettings->interfaceLanes = captureParams->csiLanes;
    /* pixel frequency is from imgDevPropery,
     * it is calculated by (VTS * HTS * FrameRate) * n sensors */
    icpSettings->pixelFrequency = property->pixelFrequency;

    /* Set SurfaceAllocAttrs */
    ctx->surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
    ctx->surfAllocAttrs[0].value = icpSettings->width;
    ctx->surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
    ctx->surfAllocAttrs[1].value = icpSettings->height;
    ctx->surfAllocAttrs[2].type = NVM_SURF_ATTR_EMB_LINES_TOP;
    ctx->surfAllocAttrs[2].value = property->embLinesTop;
    ctx->surfAllocAttrs[3].type = NVM_SURF_ATTR_EMB_LINES_BOTTOM;
    ctx->surfAllocAttrs[3].value = property->embLinesBottom;
    ctx->surfAllocAttrs[4].type = NVM_SURF_ATTR_CPU_ACCESS;
    ctx->surfAllocAttrs[4].value = NVM_SURF_ATTR_CPU_ACCESS_CACHED;
    ctx->surfAllocAttrs[5].type = NVM_SURF_ATTR_ALLOC_TYPE;
    ctx->surfAllocAttrs[5].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
    ctx->numSurfAllocAttrs = 6;
    if (testArgs->cross == STANDALONE_CROSS_PART) {
        ctx->surfAllocAttrs[6].type = NVM_SURF_ATTR_PEER_VM_ID;
        ctx->surfAllocAttrs[6].value = ctx->consumervm;
        ctx->numSurfAllocAttrs += 1;
    }

    return NVMEDIA_STATUS_OK;
}

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
_CaptureThreadFunc(
            void *data)
{
    CaptureThreadCtx *threadCtx = (CaptureThreadCtx *)data;
    uint32_t i = 0, totalCapturedFrames = 0, lastCapturedFrame = 0;
    NvMediaImage *capturedImage = NULL;
    NvMediaImage *feedImage = NULL;
    NvMediaStatus status;
    unsigned long long tbegin = 0, tend = 0, fps, td;
    NvMediaICP *icpInst = NULL;
    uint32_t retry = 0;

    if (threadCtx->icpExCtx->icp[CAPTURE_SENSOR_ID].virtualGroupId ==
                threadCtx->virtualGroupIndex) {
        icpInst = NVMEDIA_ICP_HANDLER(threadCtx->icpExCtx, CAPTURE_SENSOR_ID);
    }

    if (!icpInst) {
        LOG_ERR("%s: Failed to get icpInst for virtual channel %d\n", __func__,
                threadCtx->virtualGroupIndex);
        goto done;
    }

    while (!(*threadCtx->quit)) {

        /* Set current frame to be an offset by frames to skip */
        threadCtx->currentFrame = i;

        /* Feed all images to image capture object from the input Queue */
        while (NvQueueGet(threadCtx->inputQueue,
                          &feedImage,
                          0) == NVMEDIA_STATUS_OK) {
            status = NvMediaICPFeedFrame(icpInst,
                                         feedImage,
                                         CAPTURE_FEED_FRAME_TIMEOUT);
            if (status != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: %d: NvMediaICPFeedFrame failed\n", __func__, __LINE__);
                if (NvQueuePut((NvQueue *)feedImage->tag,
                               (void *)&feedImage,
                               0) != NVMEDIA_STATUS_OK) {
                    LOG_ERR("%s: Failed to put image back into capture input queue", __func__);
                    *threadCtx->quit = NVMEDIA_TRUE;
                    status = NVMEDIA_STATUS_ERROR;
                    goto done;
                }
                feedImage = NULL;
                *threadCtx->quit = NVMEDIA_TRUE;
                goto done;
            }
            feedImage = NULL;
        }

        /* Get captured frame */
        status = NvMediaICPGetFrameEx(icpInst,
                                      CAPTURE_GET_FRAME_TIMEOUT,
                                      &capturedImage);
        switch (status) {
            case NVMEDIA_STATUS_OK:
                retry = 0;
                break;
            case NVMEDIA_STATUS_TIMED_OUT:
                LOG_WARN("%s: NvMediaICPGetFrameEx timed out\n", __func__);
                if (++retry > CAPTURE_MAX_RETRY) {
                    LOG_ERR("%s: keep failing at NvMediaICPGetFrameEx for %d times\n",
                        __func__, retry);
                    /* Stop ICP to release all the buffer fed so far */
                    NvMediaICPStop(icpInst);
                    *threadCtx->quit = NVMEDIA_TRUE;
                    goto done;
                }
                continue;
            case NVMEDIA_STATUS_ERROR:
            default:
                LOG_ERR("%s: NvMediaICPGetFrameEx failed\n", __func__);
                *threadCtx->quit = NVMEDIA_TRUE;
                goto done;
        }

        GetTimeMicroSec(&tend);
        td = tend - tbegin;
        if (td > 3000000) {
            fps = (int)(totalCapturedFrames-lastCapturedFrame)*(1000000.0/td);

            tbegin = tend;
            lastCapturedFrame = totalCapturedFrames;
            LOG_INFO("%s: VC:%d FPS=%d delta=%lld", __func__,
                     threadCtx->virtualGroupIndex, fps, td);
        }

        /* push the captured image onto output queue */
        status = NvQueuePut(threadCtx->outputQueue,
                            (void *)&capturedImage,
                            CAPTURE_ENQUEUE_TIMEOUT);
        if (status != NVMEDIA_STATUS_OK) {
            LOG_INFO("%s: Failed to put image onto capture output queue", __func__);
            goto done;
        }

        totalCapturedFrames++;

        capturedImage = NULL;
done:
        if (capturedImage) {
            status = NvQueuePut((NvQueue *)capturedImage->tag,
                                (void *)&capturedImage,
                                0);
            if (status != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: Failed to put image back into capture input queue", __func__);
                *threadCtx->quit = NVMEDIA_TRUE;
            }
            capturedImage = NULL;
        }
        i++;

        /* To stop capturing if specified number of frames are captured */
        if (threadCtx->numFramesToCapture &&
           (totalCapturedFrames == threadCtx->numFramesToCapture))
            break;
    }

    /* Release all the frames which are fed */
    while (NvMediaICPReleaseFrame(icpInst, &capturedImage) == NVMEDIA_STATUS_OK) {
        if (capturedImage) {
            status = NvQueuePut((NvQueue *)capturedImage->tag,
                                (void *)&capturedImage,
                                0);
            if (status != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: Failed to put image back into input queue", __func__);
                break;
            }
        }
        capturedImage = NULL;
    }

    NvMediaICPStop(icpInst);

    LOG_INFO("%s: Capture thread exited\n", __func__);
    threadCtx->exitedFlag = NVMEDIA_TRUE;
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
CaptureInit(
        NvMainContext *mainCtx)
{
    NvCaptureContext *captureCtx = NULL;
    NvMediaStatus status;
    TestArgs *testArgs = mainCtx->testArgs;
    ExtImgDevParam *extImgDevParam;

    if (!mainCtx) {
        LOG_ERR("%s: Bad parameter", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    mainCtx->ctxs[CAPTURE_ELEMENT]= malloc(sizeof(NvCaptureContext));
    if (!mainCtx->ctxs[CAPTURE_ELEMENT]) {
        LOG_ERR("%s: Failed to allocate memory for capture context\n", __func__);
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    captureCtx = mainCtx->ctxs[CAPTURE_ELEMENT];
    memset(captureCtx, 0, sizeof(NvCaptureContext));

    /* initialize capture context */
    captureCtx->quit = &mainCtx->quit;
    captureCtx->testArgs  = testArgs;

    extImgDevParam = &captureCtx->extImgDevParam;

    captureCtx->captureParams = &testArgs->captureConfigCollection[
                            testArgs->config.uIntValue];

    /* Set ExtImgDev params */
    status = _SetExtImgDevParameters(captureCtx,
                captureCtx->captureParams, extImgDevParam);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to set ISC device parameters\n", __func__);
        LOG_ERR("%s: Failed to initialize Capture\n", __func__);
        return status;
    }

    /* Only Support OV10635 devices */
    if(strncmp(CAMERA_SUPPORTED, extImgDevParam->moduleName, strlen(CAMERA_SUPPORTED))) {
        LOG_ERR("%s: Currently only OV10635 camera is supported\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        LOG_ERR("%s: Failed to initialize Capture\n", __func__);
        return status;
    }

    /* Create ExtImgDev object */
    captureCtx->extImgDevice = ExtImgDevInit(extImgDevParam);
    if (!captureCtx->extImgDevice) {
        LOG_ERR("%s: Failed to initialize ISC devices\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        LOG_ERR("%s: Failed to initialize Capture\n", __func__);
        return status;
    }

    /* Create NvMedia Device */
    captureCtx->device = NvMediaDeviceCreate();
    if (!captureCtx->device) {
        status = NVMEDIA_STATUS_ERROR;
        LOG_ERR("%s: Failed to create NvMedia device\n", __func__);
        LOG_ERR("%s: Failed to initialize Capture\n", __func__);
        return status;
    }

    /* Set NvMediaICPSettingsEx */
    captureCtx->icpSettingsEx.numVirtualGroups = CAPTURE_SENSOR_NUMBER;
    captureCtx->icpSettingsEx.interfaceType = captureCtx->extImgDevice->property.interface;
    captureCtx->icpSettingsEx.interfaceLanes = captureCtx->captureParams->csiLanes;

    captureCtx->threadCtx.consumervm = captureCtx->testArgs->consumervm;
    status = _SetICPSettings(&captureCtx->threadCtx,
                            NVMEDIA_ICP_SETTINGS_HANDLER(captureCtx->icpSettingsEx,
                                    CAPTURE_SENSOR_ID, CAPTURE_VC_ID),
                            &captureCtx->extImgDevice->property,
                            captureCtx->captureParams,
                            testArgs);

    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to set ICP settings\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        LOG_ERR("%s: Failed to initialize Capture\n", __func__);
        return status;
    }
    captureCtx->icpSettingsEx.virtualGroups[CAPTURE_SENSOR_ID].numVirtualChannels =
                                                    CAPTURE_VC_NUMBER;
    captureCtx->icpSettingsEx.virtualGroups[CAPTURE_SENSOR_ID].virtualChannels[CAPTURE_VC_ID].
        virtualChannelIndex = captureCtx->extImgDevice->property.vcId[CAPTURE_SENSOR_ID];


    /* Create NvMediaICPEx object */
    captureCtx->icpExCtx = NvMediaICPCreateEx(&captureCtx->icpSettingsEx);
    if (!captureCtx->icpExCtx) {
        LOG_ERR("%s: NvMediaICPCreateEx failed\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        LOG_ERR("%s: Failed to initialize Capture\n", __func__);
        return status;
    }

    /* Create Input Queue and set data for capture thread */
    captureCtx->threadCtx.icpExCtx = captureCtx->icpExCtx;
    captureCtx->threadCtx.quit = captureCtx->quit;
    captureCtx->threadCtx.exitedFlag = NVMEDIA_TRUE;
    captureCtx->threadCtx.virtualGroupIndex = captureCtx->extImgDevice->
                                                property.vcId[CAPTURE_SENSOR_ID];
    captureCtx->threadCtx.width  = NVMEDIA_ICP_SETTINGS_HANDLER(captureCtx->icpSettingsEx,
                                                CAPTURE_SENSOR_ID, CAPTURE_VC_ID)->width;
    captureCtx->threadCtx.height = NVMEDIA_ICP_SETTINGS_HANDLER(captureCtx->icpSettingsEx,
                                                CAPTURE_SENSOR_ID, CAPTURE_VC_ID)->height;
    captureCtx->threadCtx.settings = NVMEDIA_ICP_SETTINGS_HANDLER(captureCtx->icpSettingsEx,
                                                CAPTURE_SENSOR_ID, CAPTURE_VC_ID);
    captureCtx->threadCtx.numFramesToCapture = testArgs->numFrames.uIntValue;
    captureCtx->threadCtx.numBuffers = CAPTURE_INPUT_QUEUE_SIZE;

    /* Create inputQueue for storing captured Images */
    status = _CreateImageQueue(captureCtx->device,
                                &captureCtx->threadCtx.inputQueue,
                                CAPTURE_INPUT_QUEUE_SIZE,
                                captureCtx->threadCtx.width,
                                captureCtx->threadCtx.height,
                                captureCtx->threadCtx.surfType,
                                captureCtx->threadCtx.surfAllocAttrs,
                                captureCtx->threadCtx.numSurfAllocAttrs,
                                captureCtx->threadCtx.images);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: capture InputQueue creation failed\n", __func__);
        LOG_ERR("%s: Failed to initialize Capture\n", __func__);
        return status;
    }

    LOG_DBG("%s: Capture Input Queue: %ux%u, images: %u \n",
            __func__, captureCtx->threadCtx.width,
            captureCtx->threadCtx.height,
            CAPTURE_INPUT_QUEUE_SIZE);

    /* Start ExtImgDevice */
    if (captureCtx->extImgDevice)
        ExtImgDevStart(captureCtx->extImgDevice);

    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
CaptureFini(
        NvMainContext *mainCtx)
{
    NvCaptureContext *captureCtx = NULL;
    NvMediaImage *image = NULL;
    NvMediaStatus status;
    uint32_t i = 0;

    if (!mainCtx)
        return NVMEDIA_STATUS_OK;

    captureCtx = mainCtx->ctxs[CAPTURE_ELEMENT];
    if (!captureCtx)
        return NVMEDIA_STATUS_OK;

    /* Wait for thread to exit */
    if (captureCtx->captureThread) {
        while (!captureCtx->threadCtx.exitedFlag) {
            LOG_DBG("%s: Waiting for capture thread to quit\n",
                __func__);
        }
    }

    *captureCtx->quit = NVMEDIA_TRUE;

    /* Destroy Capture thread */
    if (captureCtx->captureThread) {
        status = NvThreadDestroy(captureCtx->captureThread);
        if (status != NVMEDIA_STATUS_OK)
            LOG_ERR("%s: Failed to destroy capture thread\n",
                __func__);
    }

    /* Destroy the allocated NvMedia Images */
    for(i = 0; i < CAPTURE_INPUT_QUEUE_SIZE; i++)
    {
        image = captureCtx->threadCtx.images[i];
        if (image) {
            NvMediaImageDestroy(image);
            image = NULL;
        }
    }

    /* Destroy input queue */
    if (captureCtx->threadCtx.inputQueue) {
        LOG_DBG("%s: Destroying capture input queue %d \n", __func__, i);
        NvQueueDestroy(captureCtx->threadCtx.inputQueue);
    }

    /* Destroy contexts */
    if (captureCtx->icpExCtx)
        NvMediaICPDestroyEx(captureCtx->icpExCtx);

    if (captureCtx->device)
        NvMediaDeviceDestroy(captureCtx->device);

    if (captureCtx->extImgDevice) {
        ExtImgDevDeinit(captureCtx->extImgDevice);
    }

    if (captureCtx)
        free(captureCtx);

    LOG_INFO("%s: CaptureFini done\n", __func__);

    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
CaptureProc(
        NvMainContext *mainCtx)
{
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;
    uint32_t i=0;

    if (!mainCtx) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }

    NvCaptureContext *captureCtx  = mainCtx->ctxs[CAPTURE_ELEMENT];
    NvSaveContext    *saveCtx     = mainCtx->ctxs[SAVE_ELEMENT];

    /* Setting the queues */
    CaptureThreadCtx *threadCtx = &captureCtx->threadCtx;
    if (threadCtx) {
        threadCtx->outputQueue = saveCtx->threadCtx.inputQueue;

        threadCtx->exitedFlag = NVMEDIA_FALSE;
        status = NvThreadCreate(&captureCtx->captureThread,
                            &_CaptureThreadFunc,
                            (void *)threadCtx,
                            NV_THREAD_PRIORITY_NORMAL);
        if (status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s: Failed to create captureThread %d\n",
                __func__, i);
            threadCtx->exitedFlag = NVMEDIA_TRUE;
            return status;
        }
    }

    return status;
}
