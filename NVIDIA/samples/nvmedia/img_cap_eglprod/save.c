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

#include "capture.h"
#include "save.h"
#include "composite.h"

#define CONV_GET_X_OFFSET(xoffsets, red, green1, green2, blue) \
            xoffsets[red] = 0;\
            xoffsets[green1] = 1;\
            xoffsets[green2] = 0;\
            xoffsets[blue] = 1;

#define CONV_GET_Y_OFFSET(yoffsets, red, green1, green2, blue) \
            yoffsets[red] = 0;\
            yoffsets[green1] = 0;\
            yoffsets[green2] = 1;\
            yoffsets[blue] = 1;

#define CONV_CALCULATE_PIXEL(pSrcBuff, srcPitch, x, y, xOffset, yOffset) \
            (pSrcBuff[srcPitch*(y + yOffset) + 2*(x + xOffset) + 1] << 2) | \
            (pSrcBuff[srcPitch*(y + yOffset) + 2*(x + xOffset)] >> 6)

enum PixelColor {
    RED,
    GREEN1,
    GREEN2,
    BLUE,
    NUM_PIXEL_COLORS
};

static void
_CreateOutputFileName(
                      char *saveFilePrefix,
                      uint32_t virtualGroupIndex,
                      uint32_t frame,
                      char *outputFileName)
{
    char buf[MAX_STRING_SIZE] = {0};

    memset(outputFileName, 0, MAX_STRING_SIZE);
    strncpy(outputFileName, saveFilePrefix, MAX_STRING_SIZE);
    strcat(outputFileName, "_vc");
    sprintf(buf, "%d", virtualGroupIndex);
    strcat(outputFileName, buf);
    strcat(outputFileName, "_");
    sprintf(buf, "%02d", frame);
    strcat(outputFileName, buf);
    strcat(outputFileName, ".raw");
}

static NvMediaStatus
_ConvGetPixelOffsets(NvMediaRawPixelOrder pixelOrder,
                     NvU32 *xOffsets,
                     NvU32 *yOffsets)
{
    if (!xOffsets || !yOffsets)
        return NVMEDIA_STATUS_BAD_PARAMETER;

    switch (pixelOrder) {
        case NVMEDIA_RAW_PIXEL_ORDER_RGGB:
            CONV_GET_X_OFFSET(xOffsets, RED, GREEN1, GREEN2, BLUE);
            CONV_GET_Y_OFFSET(yOffsets, RED, GREEN1, GREEN2, BLUE);
            break;
        case NVMEDIA_RAW_PIXEL_ORDER_GRBG:
            CONV_GET_X_OFFSET(xOffsets, GREEN1, RED, BLUE, GREEN2);
            CONV_GET_Y_OFFSET(yOffsets, GREEN1, RED, BLUE, GREEN2);
            break;
        case NVMEDIA_RAW_PIXEL_ORDER_GBRG:
            CONV_GET_X_OFFSET(xOffsets, GREEN1, BLUE, RED, GREEN2);
            CONV_GET_Y_OFFSET(yOffsets, GREEN1, BLUE, RED, GREEN2);
            break;
        case NVMEDIA_RAW_PIXEL_ORDER_BGGR:
        default:
            CONV_GET_X_OFFSET(xOffsets, BLUE, GREEN1, GREEN2, RED);
            CONV_GET_Y_OFFSET(yOffsets, BLUE, GREEN1, GREEN2, RED);
            break;
    }

    return NVMEDIA_STATUS_OK;
}

static NvMediaStatus
_ConvRawToRgba(NvMediaImage *imgSrc,
               NvMediaImage *imgDst,
               NvU32 rawBytesPerPixel,
               NvU32 pixelOrder)
{
    NvMediaImageSurfaceMap surfaceMap;
    NvU32 srcImageSize = 0, srcWidth, srcHeight;
    NvU32 dstImageSize = 0, dstWidth, dstHeight;
    NvU8 *pSrcBuff = NULL, *pDstBuff = NULL, *pTmp = NULL;
    NvU32 srcPitch = 0, dstPitch = 0;
    NvMediaStatus status;
    NvU8 alpha = 0xFF;
    NvU32 x = 0, y = 0;
    NvU32 xOffsets[NUM_PIXEL_COLORS] = {0}, yOffsets[NUM_PIXEL_COLORS] = {0};
    NVM_SURF_FMT_DEFINE_ATTR(srcAttr);
    NVM_SURF_FMT_DEFINE_ATTR(dstAttr);

    status = NvMediaSurfaceFormatGetAttrs(imgSrc->type,
                                    srcAttr,
                                    NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }

    status = NvMediaSurfaceFormatGetAttrs(imgDst->type,
                                    dstAttr,
                                    NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }

    if (NvMediaImageLock(imgSrc, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap) !=
        NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageLock failed\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }

    srcHeight = surfaceMap.height;
    srcWidth  = surfaceMap.width;

    if (srcAttr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RAW) {
        srcPitch = srcWidth * rawBytesPerPixel;
        srcImageSize = srcPitch * srcHeight;
        srcImageSize += imgSrc->embeddedDataTopSize;
        srcImageSize += imgSrc->embeddedDataBottomSize;
    } else {
        LOG_ERR("%s: Unsupported source surface type\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }

    if (!(pSrcBuff = malloc(srcImageSize))) {
        LOG_ERR("%s: Out of memory\n", __func__);
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    status = NvMediaImageGetBits(imgSrc, NULL, (void **)&pSrcBuff, &srcPitch);
    NvMediaImageUnlock(imgSrc);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageGetBits() failed\n", __func__);
        goto done;
    }

    dstHeight = srcHeight / 2;
    dstWidth  = srcWidth / 2;

    if (dstAttr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
        dstPitch = dstWidth * 4;
        dstImageSize = dstHeight * dstPitch;
    } else {
        LOG_ERR("%s: Unsupported destination surface type\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    if (!(pDstBuff = calloc(1, dstImageSize))) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pTmp = pDstBuff;
    /* Convert to rgba */

    /* Y is starting at valid pixel, skipping embedded lines from top */
    y = imgSrc->embeddedDataTopSize / srcPitch;

    /* Get offsets for each pixel color */
    status = _ConvGetPixelOffsets(pixelOrder, xOffsets, yOffsets);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to get PixelOffsets\n", __func__);
        return status;
    }

    if (srcAttr[NVM_SURF_ATTR_BITS_PER_COMPONENT].value == NVM_SURF_ATTR_BITS_PER_COMPONENT_12) {
        for (; y < srcHeight; y += 2) {
            for (x = 0; x < srcWidth; x += 2) {
                /* R */
                *pTmp = CONV_CALCULATE_PIXEL(pSrcBuff, srcPitch, x, y, xOffsets[RED], yOffsets[RED]);
                pTmp++;
                /* G (average of green in BGGR) */
                *pTmp = ((CONV_CALCULATE_PIXEL(pSrcBuff, srcPitch, x, y, xOffsets[GREEN1], yOffsets[GREEN1])) +
                         (CONV_CALCULATE_PIXEL(pSrcBuff, srcPitch, x, y, xOffsets[GREEN2], yOffsets[GREEN2]))) /2 ;
                pTmp++;
                /* B */
                *pTmp = CONV_CALCULATE_PIXEL(pSrcBuff, srcPitch, x, y, xOffsets[BLUE], yOffsets[BLUE]);
                pTmp++;
                /* A */
                *pTmp = alpha;
                pTmp++;
            }
        }
    } else {
        LOG_ERR("%s: Unsupported input raw format\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        goto done;
    }
    memset(&surfaceMap, 0, sizeof(surfaceMap));

    if (NvMediaImageLock(imgDst, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap) !=
       NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageLock failed\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    status = NvMediaImagePutBits(imgDst, NULL, (void **)&pDstBuff, &dstPitch);
    NvMediaImageUnlock(imgDst);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImagePutBits() failed\n", __func__);
        goto done;
    }

    status = NVMEDIA_STATUS_OK;
done:
    if (pSrcBuff)
        free(pSrcBuff);
    if (pDstBuff)
        free(pDstBuff);

    return status;
}

static uint32_t
_SaveThreadFunc(
                void *data)
{
    SaveThreadCtx *threadCtx = (SaveThreadCtx *)data;
    NvMediaImage *image = NULL;
    NvMediaImage *convertedImage = NULL;
    NvMediaStatus status;
    uint32_t totalSavedFrames = 0;
    char outputFileName[MAX_STRING_SIZE];

    NVM_SURF_FMT_DEFINE_ATTR(attr);

    while (!(*threadCtx->quit)) {
        image=NULL;
        /* Wait for captured frames */
        while (NvQueueGet(threadCtx->inputQueue, &image, SAVE_DEQUEUE_TIMEOUT) !=
           NVMEDIA_STATUS_OK) {
            LOG_DBG("%s: saveThread input queue %d is empty\n",
                     __func__, threadCtx->virtualGroupIndex);
            if (*threadCtx->quit)
                goto loop_done;
        }

        if (threadCtx->saveEnabled) {
            /* Save image to file */
            _CreateOutputFileName(threadCtx->saveFilePrefix,
                                  threadCtx->virtualGroupIndex,
                                  totalSavedFrames,
                                  outputFileName);

            LOG_INFO("%s: Write image. res [%u:%u] (file: %s)\n",
                        __func__, image->width, image->height,
                        outputFileName);

            WriteImage(outputFileName,
                       image,
                       NVMEDIA_TRUE,
                       NVMEDIA_FALSE,
                       threadCtx->rawBytesPerPixel);
        }

        totalSavedFrames++;

        status = NvMediaSurfaceFormatGetAttrs(threadCtx->surfType,
                                                attr,
                                                NVM_SURF_FMT_ATTR_MAX);

        if (status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
            *threadCtx->quit = NVMEDIA_TRUE;
            goto loop_done;
        }

        while (NvQueuePut(threadCtx->outputQueue,
                        &image,
                        SAVE_ENQUEUE_TIMEOUT) != NVMEDIA_STATUS_OK) {
            LOG_DBG("%s: savethread output queue %d is full\n",
                __func__, threadCtx->virtualGroupIndex);
            if (*threadCtx->quit)
                goto loop_done;
        }
        image=NULL;


        if (threadCtx->numFramesToSave &&
           (totalSavedFrames == threadCtx->numFramesToSave)) {
            *threadCtx->quit = NVMEDIA_TRUE;
            goto loop_done;
        }

loop_done:
        if (image) {
            if (NvQueuePut((NvQueue *)image->tag,
                           (void *)&image,
                           0) != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: Failed to put image back in queue\n", __func__);
                *threadCtx->quit = NVMEDIA_TRUE;
            };
            image = NULL;
        }
        if (convertedImage) {
            if (NvQueuePut((NvQueue *)convertedImage->tag,
                           (void *)&convertedImage,
                           0) != NVMEDIA_STATUS_OK) {
                LOG_ERR("%s: Failed to put image back in conversionQueue\n", __func__);
                *threadCtx->quit = NVMEDIA_TRUE;
            }
            convertedImage = NULL;
        }
    }
    LOG_INFO("%s: Save thread exited\n", __func__);
    threadCtx->exitedFlag = NVMEDIA_TRUE;
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
SaveInit(
        NvMainContext *mainCtx)
{
    NvSaveContext *saveCtx  = NULL;
    NvCaptureContext   *captureCtx = NULL;
    TestArgs           *testArgs = mainCtx->testArgs;
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;

    /* allocating save context */
    mainCtx->ctxs[SAVE_ELEMENT]= malloc(sizeof(NvSaveContext));
    if (!mainCtx->ctxs[SAVE_ELEMENT]){
        LOG_ERR("%s: Failed to allocate memory for save context\n", __func__);
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    saveCtx = mainCtx->ctxs[SAVE_ELEMENT];
    memset(saveCtx,0,sizeof(NvSaveContext));
    captureCtx = mainCtx->ctxs[CAPTURE_ELEMENT];

    /* initialize context */
    saveCtx->quit      =  &mainCtx->quit;
    saveCtx->testArgs  = testArgs;
    /* Create NvMedia Device */
    saveCtx->device = NvMediaDeviceCreate();
    if (!saveCtx->device) {
        status = NVMEDIA_STATUS_ERROR;
        LOG_ERR("%s: Failed to create NvMedia device\n", __func__);
        goto failed;
    }


    /* Create save input Queues and set thread data */
    saveCtx->threadCtx.quit = saveCtx->quit;
    saveCtx->threadCtx.exitedFlag = NVMEDIA_TRUE;
    saveCtx->threadCtx.saveEnabled = testArgs->filePrefix.isUsed;
    saveCtx->threadCtx.saveFilePrefix = testArgs->filePrefix.stringValue;
    saveCtx->threadCtx.virtualGroupIndex = captureCtx->threadCtx[0].virtualGroupIndex;
    saveCtx->threadCtx.numFramesToSave = testArgs->numFrames.uIntValue;
    saveCtx->threadCtx.surfType = captureCtx->threadCtx[0].surfType;
    saveCtx->threadCtx.pixelOrder = captureCtx->threadCtx[0].pixelOrder;
    saveCtx->threadCtx.rawBytesPerPixel = captureCtx->threadCtx[0].rawBytesPerPixel;

    NVM_SURF_FMT_DEFINE_ATTR(attr);
    status = NvMediaSurfaceFormatGetAttrs(captureCtx->threadCtx[0].surfType,
                                        attr,
                                        NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        goto failed;
    }
    saveCtx->threadCtx[i].width =  (attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RAW )?
                                       captureCtx->threadCtx[i].width/2 : captureCtx->threadCtx[i].width;
    saveCtx->threadCtx[i].height = (attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RAW )?
                                       captureCtx->threadCtx[i].height/2 : captureCtx->threadCtx[i].height;
    if (NvQueueCreate(&saveCtx->threadCtx.inputQueue,
                    SAVE_QUEUE_SIZE,
                    sizeof(NvMediaImage *)) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to create save inputQueue %d\n",
                __func__);
        status = NVMEDIA_STATUS_ERROR;
        goto failed;
    }

    return NVMEDIA_STATUS_OK;
failed:
    LOG_ERR("%s: Failed to initialize Save\n",__func__);
    return status;
}

NvMediaStatus
SaveFini(
        NvMainContext *mainCtx)
{
    NvSaveContext *saveCtx = NULL;
    NvMediaImage *image = NULL;
    NvMediaStatus status = NVMEDIA_STATUS_OK;

    if (!mainCtx)
        return NVMEDIA_STATUS_OK;

    saveCtx = mainCtx->ctxs[SAVE_ELEMENT];
    if (!saveCtx)
        return NVMEDIA_STATUS_OK;

    /* Wait for thread to exit */
    if (saveCtx->saveThread) {
        while (!saveCtx->threadCtx.exitedFlag) {
            LOG_DBG("%s: Waiting for save thread to quit\n",
                __func__);
        }
    }


    *saveCtx->quit = NVMEDIA_TRUE;

    /* Destroy threads */
    if (saveCtx->saveThread) {
        status = NvThreadDestroy(saveCtx->saveThread);
        if (status != NVMEDIA_STATUS_OK)
            LOG_ERR("%s: Failed to destroy save thread\n",
                __func__);
    }

    /*For RAW Images, destroy the conversion queue */
    if (saveCtx->threadCtx.conversionQueue) {
        while (IsSucceed(NvQueueGet(saveCtx->threadCtx.conversionQueue, &image, 0))) {
            if (image) {
                NvMediaImageDestroy(image);
                image = NULL;
            }
        }
        LOG_DBG("%s: Destroying conversion queue \n",__func__);
        NvQueueDestroy(saveCtx->threadCtx.conversionQueue);
    }

    /*Flush and destroy the input queues*/
    if (saveCtx->threadCtx.inputQueue) {
        LOG_DBG("%s: Flushing the save input queue\n", __func__);
        while (IsSucceed(NvQueueGet(saveCtx->threadCtx.inputQueue, &image, 0))) {
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
        NvQueueDestroy(saveCtx->threadCtx.inputQueue);
    }

    if (saveCtx->device)
        NvMediaDeviceDestroy(saveCtx->device);

    if (saveCtx)
        free(saveCtx);

    LOG_INFO("%s: SaveFini done\n", __func__);
    return NVMEDIA_STATUS_OK;
}


NvMediaStatus
SaveProc(
        NvMainContext *mainCtx)
{
    NvSaveContext        *saveCtx = NULL;
    NvCompositeContext   *compositeCtx = NULL;
    NvMediaStatus status= NVMEDIA_STATUS_OK;

    if (!mainCtx) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }
    saveCtx = mainCtx->ctxs[SAVE_ELEMENT];
    compositeCtx = mainCtx->ctxs[COMPOSITE_ELEMENT];

    /* Setting the queues */
    saveCtx->threadCtx.outputQueue = compositeCtx->inputQueue;

    /* Create thread to save images */
    saveCtx->threadCtx.exitedFlag = NVMEDIA_FALSE;
    status = NvThreadCreate(&saveCtx->saveThread,
                            &_SaveThreadFunc,
                            (void *)&saveCtx->threadCtx,
                            NV_THREAD_PRIORITY_NORMAL);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to create save Thread\n",
                __func__);
        saveCtx->threadCtx.exitedFlag = NVMEDIA_TRUE;
    }

    return status;
}
