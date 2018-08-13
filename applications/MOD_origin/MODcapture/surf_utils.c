/*
 * Copyright (c) 2013-2017 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log_utils.h"
#include "surf_utils.h"

#define MAXM_NUM_SURFACES 6

typedef struct {
    float heightFactor[6];
    float widthFactor[6];
    unsigned int numSurfaces;
} utilSurfParams;

utilSurfParams surfParamsTable_RGBA  = {
    .heightFactor = {1, 0, 0, 0, 0, 0},
    .widthFactor = {1, 0, 0, 0, 0, 0},
    .numSurfaces = 1,
};

utilSurfParams surfParamsTable_RAW  = {
    .heightFactor = {1, 0, 0, 0, 0, 0},
    .widthFactor = {1, 0, 0, 0, 0, 0},
    .numSurfaces = 1,
};

utilSurfParams surfParamsTable_YUV[][4] = {
    { /* PLANAR */
        { /* 420 */
            .heightFactor = {1, 0.5, 0.5, 0, 0, 0},
            .widthFactor = {1, 0.5, 0.5, 0, 0, 0},
            .numSurfaces = 3,
        },
        { /* 422 */
            .heightFactor = {1, 1, 1, 0, 0, 0},
            .widthFactor = {1, 0.5, 0.5, 0, 0, 0},
            .numSurfaces = 3,
        },
        { /* 444 */
            .heightFactor = {1, 1, 1, 0, 0, 0},
            .widthFactor = {1, 1, 1, 0, 0, 0},
            .numSurfaces = 3,
        },
        { /* 422R */
            .heightFactor = {1, 0.5, 0.5, 0, 0, 0},
            .widthFactor = {1, 1, 1, 0, 0, 0},
            .numSurfaces = 3,
        },
    },
    { /* SEMI_PLANAR */
        { /* 420 */
            .heightFactor = {1, 0.5, 0, 0, 0, 0},
            .widthFactor = {1, 0.5, 0, 0, 0, 0},
            .numSurfaces = 2,
        },
        { /* 422 */
            .heightFactor = {1, 1, 0, 0, 0, 0},
            .widthFactor = {1, 0.5, 0, 0, 0, 0},
            .numSurfaces = 2,
        },
        { /* 444 */
            .heightFactor = {1, 1, 0.5, 0, 0, 0},
            .widthFactor = {1, 1, 0.5, 0, 0, 0},
            .numSurfaces = 2,
        },
        { /* 422R */
            .heightFactor = {1, 0.5, 0.5, 0, 0, 0},
            .widthFactor = {1, 1, 0.5, 0, 0, 0},
            .numSurfaces = 2,
        },
    },
    { /* PACKED */
        { /* 420 */
            .heightFactor = {1, 0.5, 0.5, 0, 0, 0},
            .widthFactor = {1, 0.5, 0.5, 0, 0, 0},
            .numSurfaces = 3,
        },
        { /* 422 */
            .heightFactor = {1, 1, 1, 0, 0, 0},
            .widthFactor = {1, 0.5, 0.5, 0, 0, 0},
            .numSurfaces = 3,
        },
        { /* 444 */
            .heightFactor = {1, 1, 1, 0, 0, 0},
            .widthFactor = {1, 1, 1, 0, 0, 0},
            .numSurfaces = 3,
        },
        { /* 422R */
            .heightFactor = {1, 0.5, 0.5, 0, 0, 0},
            .widthFactor = {1, 1, 1, 0, 0, 0},
            .numSurfaces = 3,
        },
    },
};

utilSurfParams surfParamsTable_Packed  = {
    .heightFactor = {1, 0, 0, 0, 0, 0},
    .widthFactor = {1, 0, 0, 0, 0, 0},
    .numSurfaces = 1,
};


unsigned int bytesPerPixelTable_RGBA[][6] = {
    {4, 0, 0, 0, 0, 0}, /* 8 */
};
unsigned int bytesPerPixelTable_Alpha[][6] = {
    {1, 0, 0, 0, 0, 0}, /* 8 */
    {2, 0, 0, 0, 0, 0}, /* 10 */
    {2, 0, 0, 0, 0, 0}, /* 12 */
    {2, 0, 0, 0, 0, 0}, /* 14 */
    {2, 0, 0, 0, 0, 0}, /* 16 */
    {4, 0, 0, 0, 0, 0}, /* 32 */
};

unsigned int bytesPerPixelTable_RAW[][6] = {
    {1, 0, 0, 0, 0, 0}, /* 8 */
    {2, 0, 0, 0, 0, 0}, /* 10 */
    {2, 0, 0, 0, 0, 0}, /* 12 */
    {2, 0, 0, 0, 0, 0}, /* 14 */
    {2, 0, 0, 0, 0, 0}, /* 16 */
};

unsigned int bytesPerPixelTable_YUV[][9][6] = {
    { /* PLANAR */
        {1, 1, 1, 0, 0, 0}, /* 8 */
        {2, 2, 2, 0, 0, 0}, /* 10 */
        {2, 2, 2, 0, 0, 0}, /* 12 */
        {2, 2, 2, 0, 0, 0}, /* 14 */
        {2, 2, 2, 0, 0, 0}, /* 16 */
        {4, 4, 4, 0, 0, 0}, /* 32 */
        {2, 1, 1, 0, 0, 0}, /* 16_8_8 */
        {2, 1, 1, 0, 0, 0}, /* 10_8_8 */
        {4, 0, 0, 0, 0, 0}, /* 2_10_10_10 */
    },
    { /* SEMI_PLANAR */
        {1, 2, 0, 0, 0, 0}, /* 8 */
        {2, 4, 0, 0, 0, 0}, /* 10 */
        {2, 4, 0, 0, 0, 0}, /* 12 */
        {2, 4, 0, 0, 0, 0}, /* 14 */
        {2, 4, 0, 0, 0, 0}, /* 16 */
        {4, 8, 0, 0, 0, 0}, /* 32 */
        {2, 2, 0, 0, 0, 0}, /* 16_8_8 */
        {2, 2, 0, 0, 0, 0}, /* 10_8_8 */
        {4, 0, 0, 0, 0, 0}, /* 2_10_10_10 */
    },
    { /* PACKED */
        {1, 1, 1, 0, 0, 0}, /* 8 */
        {2, 2, 2, 0, 0, 0}, /* 10 */
        {2, 2, 2, 0, 0, 0}, /* 12 */
        {2, 2, 2, 0, 0, 0}, /* 14 */
        {2, 2, 2, 0, 0, 0}, /* 16 */
        {4, 4, 4, 0, 0, 0}, /* 32 */
        {2, 1, 1, 0, 0, 0}, /* 16_8_8 */
        {2, 1, 1, 0, 0, 0}, /* 10_8_8 */
        {4, 0, 0, 0, 0, 0}, /* 2_10_10_10 */
    },
};

static NvMediaStatus
GetSurfParams(unsigned int surfaceType,
             float **xScale,
             float **yScale,
             unsigned int **bytePerPixel,
             NvU32 *numSurfacesVal)
{
    NvMediaStatus status;
    unsigned int surfType, surfMemoryType, surfSubSamplingType, surfBPC, surfCompOrder;
    float *xScalePtr = NULL, *yScalePtr = NULL;
    unsigned int *bytePerPixelPtr = NULL;
    NVM_SURF_FMT_DEFINE_ATTR(srcAttr);
    NvU32 numSurfaces = 1;

    status = NvMediaSurfaceFormatGetAttrs(surfaceType,
                                          srcAttr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }

    surfType = srcAttr[NVM_SURF_ATTR_SURF_TYPE].value;
    surfMemoryType = srcAttr[NVM_SURF_ATTR_MEMORY].value;
    surfSubSamplingType = srcAttr[NVM_SURF_ATTR_SUB_SAMPLING_TYPE].value;
    surfBPC = srcAttr[NVM_SURF_ATTR_BITS_PER_COMPONENT].value;
    surfCompOrder = srcAttr[NVM_SURF_ATTR_COMPONENT_ORDER].value;

    switch(surfType) {
        case NVM_SURF_ATTR_SURF_TYPE_YUV:
            if (surfSubSamplingType == 0 && surfMemoryType == NVM_SURF_ATTR_MEMORY_PACKED) {
                xScalePtr =  &surfParamsTable_Packed.widthFactor[0];
                yScalePtr = &surfParamsTable_Packed.heightFactor[0];
                numSurfaces = surfParamsTable_Packed.numSurfaces;
            } else {
                xScalePtr = &surfParamsTable_YUV[0][surfSubSamplingType - NVM_SURF_ATTR_SUB_SAMPLING_TYPE_420].widthFactor[0];
                yScalePtr = &surfParamsTable_YUV[0][surfSubSamplingType - NVM_SURF_ATTR_SUB_SAMPLING_TYPE_420].heightFactor[0];
                numSurfaces = surfParamsTable_YUV[0][surfSubSamplingType - NVM_SURF_ATTR_SUB_SAMPLING_TYPE_420].numSurfaces;
            }
            bytePerPixelPtr = &bytesPerPixelTable_YUV[0][surfBPC - NVM_SURF_ATTR_BITS_PER_COMPONENT_8][0];
            break;
        case NVM_SURF_ATTR_SURF_TYPE_RGBA:
            if (surfCompOrder == NVM_SURF_ATTR_COMPONENT_ORDER_ALPHA) {
                bytePerPixelPtr = &bytesPerPixelTable_Alpha[surfBPC - NVM_SURF_ATTR_BITS_PER_COMPONENT_8][0];
            } else {
                bytePerPixelPtr = &bytesPerPixelTable_RGBA[0][0];
            }
            xScalePtr = &surfParamsTable_RGBA.widthFactor[0];
            yScalePtr = &surfParamsTable_RGBA.heightFactor[0];
            numSurfaces =  surfParamsTable_RGBA.numSurfaces;
            break;
        case NVM_SURF_ATTR_SURF_TYPE_RAW:
            bytePerPixelPtr = &bytesPerPixelTable_RAW[surfBPC - NVM_SURF_ATTR_BITS_PER_COMPONENT_8][0];
            xScalePtr = &surfParamsTable_RAW.widthFactor[0];
            yScalePtr = &surfParamsTable_RAW.heightFactor[0];
            numSurfaces =  surfParamsTable_RAW.numSurfaces;
            break;
        default:
            LOG_ERR("%s: Unsupported Pixel Format %d", __func__, surfType);
            return NVMEDIA_STATUS_ERROR;
    }

    if (xScale) {
        *xScale = xScalePtr;
    }
    if (yScale) {
        *yScale = yScalePtr;
    }
    if (bytePerPixel) {
        *bytePerPixel = bytePerPixelPtr;
    }
    if (numSurfacesVal) {
        *numSurfacesVal = numSurfaces;
    }

    return NVMEDIA_STATUS_OK;
}

static NvMediaStatus
WriteFrameOld(
    char *filename,
    NvMediaVideoSurface *videoSurface,
    NvMediaBool bOrderUV,
    NvMediaBool bAppend)
{
    static int surfCount = 0;
    NvMediaVideoSurfaceMap surfaceMap;
    unsigned char *pBuff = NULL, *pDstBuff[3] = {NULL}, *pChroma;
    unsigned int dstPitches[3];
    unsigned int FrameSize = 0, width, height;
    int indexU = 1, indexV = 2;
    int bytePerPixel = 1;
    NvMediaStatus status, result = NVMEDIA_STATUS_ERROR;
    int shift = 0; // 16 - bitdepth
    unsigned int i;
    NvU16 *psrc;
    unsigned int size;
    int xScale = 2, yScale = 2;
    FILE *file = NULL;
    if(!videoSurface || !filename) {
        LOG_ERR("WriteFrame: Bad parameter\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    width = videoSurface->width;
    height = videoSurface->height;

    switch(videoSurface->type) {
        case NvMediaSurfaceType_Video_420_10bit:
        case NvMediaSurfaceType_Video_422_10bit:
        case NvMediaSurfaceType_Video_444_10bit:
            shift = 6;
            bytePerPixel = 2;
            break;
        case NvMediaSurfaceType_Video_420_12bit:
        case NvMediaSurfaceType_Video_422_12bit:
        case NvMediaSurfaceType_Video_444_12bit:
            shift = 4;
            bytePerPixel = 2;
            break;
        default:
            bytePerPixel = 1;
            break;
    }

    file = fopen(filename, bAppend ? "ab" : "wb");
    if(!file) {
        LOG_ERR("WriteFrame: file open failed: %s\n", filename);
        perror(NULL);
        return NVMEDIA_STATUS_ERROR;
    }
    surfCount++;

    pBuff = (unsigned char*)malloc(width * height * bytePerPixel * 4);
    if(!pBuff) {
        LOG_ERR("WriteFrame: Failed to allocate image buffer\n");
        fclose(file);
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    pDstBuff[0] = pBuff;

    switch(videoSurface->type) {
        case NvMediaSurfaceType_YV24:
        case NvMediaSurfaceType_Video_444_10bit:
        case NvMediaSurfaceType_Video_444_12bit:
            xScale = 1;
            yScale = 1;
            pDstBuff[indexV] = pDstBuff[0] + width * height * bytePerPixel;
            pDstBuff[indexU] = pDstBuff[indexV] + width * height * bytePerPixel;
            dstPitches[0] = dstPitches[indexU] = dstPitches[indexV] = width * bytePerPixel;
            break;
        case NvMediaSurfaceType_YV16:
        case NvMediaSurfaceType_YV16x2:
        case NvMediaSurfaceType_Video_422_10bit:
        case NvMediaSurfaceType_Video_422_12bit:
            xScale = 1;
            yScale = 2;
            pDstBuff[indexV] = pDstBuff[0] + width * height * bytePerPixel;
            pDstBuff[indexU] = pDstBuff[indexV] + width * height * bytePerPixel / 2;
            dstPitches[0] = width * bytePerPixel;
            dstPitches[indexU] = dstPitches[indexV] = width * bytePerPixel / 2;
            break;
        case NvMediaSurfaceType_YV12:
        case NvMediaSurfaceType_Video_420_10bit:
        case NvMediaSurfaceType_Video_420_12bit:
            pDstBuff[indexV] = pDstBuff[0] + width * height * bytePerPixel;
            pDstBuff[indexU] = pDstBuff[indexV] + width * height * bytePerPixel / 4;
            dstPitches[0] = width * bytePerPixel;
            dstPitches[indexU] = dstPitches[indexV] = width * bytePerPixel / 2;
            break;
        case NvMediaSurfaceType_R8G8B8A8:
        case NvMediaSurfaceType_R8G8B8A8_BottomOrigin:
            FrameSize = width * height * bytePerPixel* 4;
            dstPitches[0] = width * bytePerPixel * 4;
            break;
        default:
            LOG_ERR("WriteFrame: Invalid video surface type %d\n", videoSurface->type);
            goto done;
    }

    NvMediaVideoSurfaceLock(videoSurface, &surfaceMap);
    LOG_DBG("WriteFrame: %s Size: %dx%d Luma pitch: %d Chroma pitch: %d Chroma type: %d\n",
            filename, surfaceMap.lumaWidth, surfaceMap.lumaHeight, surfaceMap.pitchY, surfaceMap.pitchU, videoSurface->type);
    status = NvMediaVideoSurfaceGetBits(videoSurface, NULL, (void **)pDstBuff, dstPitches);
    NvMediaVideoSurfaceUnlock(videoSurface);

    if(status) {
        LOG_ERR("WriteFrame: NvMediaVideoSurfaceGetBits() failed\n");
        goto done;
    }

    // nvmedia 10bit format [a7a600 0000 b7b6b5b4b3b2b1b0] b0-b7 occupies higher bits, a6 - a7 occupies lower bits
    // yuv 10bit format [b5b4b3b2b1b0a7a6 000000b7b6]
    // Example: nvmedia 10bit format 0x40 0x62 -> yuv 10bit 0x89 0x01
    if(videoSurface->type == NvMediaSurfaceType_Video_420_10bit ||
       videoSurface->type == NvMediaSurfaceType_Video_420_12bit ||
       videoSurface->type == NvMediaSurfaceType_Video_422_10bit ||
       videoSurface->type == NvMediaSurfaceType_Video_422_12bit ||
       videoSurface->type == NvMediaSurfaceType_Video_444_10bit ||
       videoSurface->type == NvMediaSurfaceType_Video_444_12bit) {
       // Y
       psrc = (NvU16*)(pDstBuff[0]);
       size = width * height;
       for(i = 0; i < size; i++) {
            *(psrc + i) = (*(psrc + i)) >> shift;
       }

       // U
       psrc = (NvU16*)(pDstBuff[indexU]);
       size = width * height / (xScale * yScale);
       for(i = 0; i < size; i++) {
            *(psrc + i) = (*(psrc + i)) >> shift;
       }

       // V
       psrc = (NvU16*)(pDstBuff[indexV]);
       size = width * height / (xScale * yScale);
       for(i = 0; i < size; i++) {
            *(psrc + i) = (*(psrc + i)) >> shift;
       }
    }

    switch(videoSurface->type) {
        case NvMediaSurfaceType_YV24:
        case NvMediaSurfaceType_Video_444_10bit:
        case NvMediaSurfaceType_Video_444_12bit:
        case NvMediaSurfaceType_YV16:
        case NvMediaSurfaceType_YV16x2:
        case NvMediaSurfaceType_Video_422_10bit:
        case NvMediaSurfaceType_Video_422_12bit:
        case NvMediaSurfaceType_YV12:
        case NvMediaSurfaceType_Video_420_10bit:
        case NvMediaSurfaceType_Video_420_12bit:
            if(fwrite(pDstBuff[0], width * height * bytePerPixel, 1, file) != 1) {
                LOG_ERR("WriteFrame: file write failed\n");
                goto done;
            }
            pChroma = bOrderUV ? pDstBuff[indexU] : pDstBuff[indexV];
            if(fwrite(pChroma, width * height * bytePerPixel / (xScale * yScale), 1, file) != 1) {
                LOG_ERR("WriteFrame: file write failed\n");
                goto done;
            }
            pChroma = bOrderUV ? pDstBuff[indexV] : pDstBuff[indexU];
            if(fwrite(pChroma, width * height * bytePerPixel / (xScale * yScale), 1, file) != 1) {
                LOG_ERR("WriteFrame: file write failed\n");
                goto done;
            }
            break;
        case NvMediaSurfaceType_R8G8B8A8:
        case NvMediaSurfaceType_R8G8B8A8_BottomOrigin:
            if(fwrite(pDstBuff[0], FrameSize, 1, file) != 1) {
                LOG_ERR("WriteFrame: file write failed\n");
                goto done;
            }
            break;
        default:
            LOG_ERR("WriteFrame: Invalid video surface type %d\n", videoSurface->type);
            goto done;
    }

    result = NVMEDIA_STATUS_OK;

done:
    if(pBuff)
        free(pBuff);

    if(file)
        fclose(file);

    return result;
}

static NvMediaStatus
WriteFrameNew(
    char *filename,
    NvMediaVideoSurface *videoSurface,
    NvMediaBool bOrderUV,
    NvMediaBool bAppend)
{
    NvU8 **pBuff = NULL;
    NvU32 *pBuffPitches = NULL;
    NvU32 imageSize = 0;
    unsigned int size[3] ={0};
    NvU8 *buffer = NULL;
    NvU32 i, k, newk = 0;
    float *xScalePtr = NULL, *yScalePtr = NULL;
    unsigned int *bytePerPixelPtr = NULL;
    NvU32 numSurfaces = 1;
    unsigned int height, width;
    NvMediaVideoSurfaceMap surfaceMap;
    NvMediaStatus status;
    FILE *file = NULL;

    if(!videoSurface || !filename) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    height = videoSurface->height;
    width  = videoSurface->width;

    file = fopen(filename, bAppend ? "ab" : "wb");
    if(!file) {
        LOG_ERR("%s: File open failed: %s\n", __func__, filename);
        status = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    pBuff = (NvU8**)(malloc(sizeof(NvU8*) * MAXM_NUM_SURFACES));
    if(!pBuff) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pBuffPitches = (NvU32*)calloc(1, sizeof(NvU32) * MAXM_NUM_SURFACES);
    if(!pBuffPitches) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    status = GetSurfParams(videoSurface->type,
                           &xScalePtr,
                           &yScalePtr,
                           &bytePerPixelPtr,
                           &numSurfaces);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: GetSurfParams() failed\n", __func__);
        goto done;
    }

    imageSize = 0;
    for(i = 0; i < numSurfaces; i++) {
        size[i] = width * xScalePtr[i] * height * yScalePtr[i] * bytePerPixelPtr[i];
        imageSize += size[i];
        pBuffPitches[i] = (NvU32)((float)width * xScalePtr[i]) * bytePerPixelPtr[i];
    }

    buffer = (NvU8*)calloc(1, imageSize);
    if(!buffer) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    memset(buffer, 0xFF,imageSize);
    pBuff[0] = buffer;
    for(i = 1; i < numSurfaces; i++) {
        pBuff[i] = pBuff[i - 1] + (NvU32)(height * yScalePtr[i - 1] * pBuffPitches[i - 1]);
    }

    LOG_DBG("WriteFrame: %s Size: %dx%d Luma pitch: %d Chroma pitch: %d Chroma type: %d\n",
            filename, surfaceMap.lumaWidth, surfaceMap.lumaHeight, surfaceMap.pitchY, surfaceMap.pitchU, videoSurface->type);

    status = NvMediaVideoSurfaceLock(videoSurface, &surfaceMap);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaVideoSurfaceLock() failed\n", __func__);
        goto done;
    }
    status = NvMediaVideoSurfaceGetBits(videoSurface, NULL, (void **)pBuff, pBuffPitches);
    NvMediaVideoSurfaceUnlock(videoSurface);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaVideoSurfaceGetBits() failed\n", __func__);
        goto done;
    }

    for(k = 0; k < numSurfaces; k++) {
       newk = (!bOrderUV && k ) ? (numSurfaces - k) : k;
       if (fwrite(pBuff[newk], size[newk], 1, file) != 1) {
           LOG_ERR("%s: File write failed\n", __func__);
           status = NVMEDIA_STATUS_ERROR;
           goto done;
       }
    }

done:
    if(file) {
        fclose(file);
    }

    if(pBuff) {
        free(pBuff);
    }

    if(buffer) {
        free(buffer);
    }

    if(pBuffPitches) {
        free(pBuffPitches);
    }

    return status;
}


NvMediaStatus
WriteFrame(
    char *filename,
    NvMediaVideoSurface *videoSurface,
    NvMediaBool bOrderUV,
    NvMediaBool bAppend)
{
    NvMediaStatus status;
    NVM_SURF_FMT_DEFINE_ATTR(srcAttr);

    if(!videoSurface || !filename) {
        LOG_ERR("WriteFrame: Bad parameter\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    status = NvMediaSurfaceFormatGetAttrs(videoSurface->type,
                                          srcAttr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status == NVMEDIA_STATUS_NOT_SUPPORTED) {
        return WriteFrameOld(
                         filename,
                         videoSurface,
                         bOrderUV,
                         bAppend);
    } else if (status == NVMEDIA_STATUS_OK) {
        return WriteFrameNew(
                         filename,
                         videoSurface,
                         bOrderUV,
                         bAppend);
    } else {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return status;
    }
}

static NvMediaStatus
ReadFrameOld(
    char *fileName,
    NvU32 uFrameNum,
    NvU32 uWidth,
    NvU32 uHeight,
    NvMediaVideoSurface *pFrame,
    NvMediaBool bOrderUV)
{
    NvMediaStatus ret = NVMEDIA_STATUS_OK;
    if(pFrame->type == NvMediaSurfaceType_Video_420) {
        ret = ReadYUVFrame(fileName, uFrameNum, uWidth,
                           uHeight, pFrame, bOrderUV);
    } else if(pFrame->type == NvMediaSurfaceType_R8G8B8A8_BottomOrigin) {
        ret = ReadRGBAFrame(fileName, uFrameNum, uWidth,
                            uHeight, pFrame);
    } else {
        LOG_ERR("ReadFrame: Invalid video surface type %d\n", pFrame->type);
        ret = NVMEDIA_STATUS_ERROR;
    }
    return ret;
}

NvMediaStatus
ReadYUVFrame(
    char *fileName,
    NvU32 uFrameNum,
    NvU32 uWidth,
    NvU32 uHeight,
    NvMediaVideoSurface *pFrame,
    NvMediaBool bOrderUV)
{
    NvU8 *pBuff = NULL, *pYBuff, *pUBuff, *pVBuff, *pChroma;
    FILE *file = NULL;
    NvU32 uFrameSize = 0;
    NvMediaVideoSurfaceMap surfaceMap;
    NvU32 uHeightSurface, uWidthSurface, uSurfaceSize;
    NvMediaStatus ret = NVMEDIA_STATUS_OK;
    NvU8 *pYUVBuff[3];
    NvU32 YUVPitch[3];
    unsigned int i;
    int xScale = 2, yScale = 2;
    int bpp = 1;
    int shift = 0; // 16 - bitdepth
    NvU16 *psrc;
    unsigned int size;

    NvMediaVideoSurfaceLock(pFrame, &surfaceMap);

    uHeightSurface = surfaceMap.lumaHeight;
    uWidthSurface  = surfaceMap.lumaWidth;

    switch(pFrame->type) {
    case NvMediaSurfaceType_YV24:
        uSurfaceSize = uHeightSurface * uWidthSurface * 3;
        uFrameSize = uWidth * uHeight * 3;
        xScale = 1;
        yScale = 1;
        break;
    case NvMediaSurfaceType_YV12:
        uSurfaceSize = (uHeightSurface * uWidthSurface * 3) / 2;
        uFrameSize = (uWidth * uHeight * 3) / 2;
        break;
    case NvMediaSurfaceType_Video_420_10bit:
        shift = 6;
        bpp = 2;
        uSurfaceSize = (uHeightSurface * uWidthSurface * 3 * bpp) / 2;
        uFrameSize = (uWidth * uHeight * 3 * bpp) / 2;
        break;
    case NvMediaSurfaceType_Video_444_10bit:
        xScale = 1;
        yScale = 1;
        shift = 6;
        bpp = 2;
        uSurfaceSize = uHeightSurface * uWidthSurface * 3 * bpp;
        uFrameSize = uWidth * uHeight * 3 * bpp;
        break;
    default:
        LOG_ERR("ReadYUVFrame: Invalid video surface type %d\n", pFrame->type);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    if(!pFrame || uWidth > uWidthSurface || uHeight > uHeightSurface)
        return NVMEDIA_STATUS_BAD_PARAMETER;

    pBuff = (NvU8*)malloc(uSurfaceSize);
    if(!pBuff) {
        LOG_ERR("ReadYUVFrame: Failed to allocate image buffer\n");
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    memset(pBuff, 0x10, uWidthSurface * uHeightSurface * bpp);
    memset(pBuff + uWidthSurface * uHeightSurface * bpp, 0x80, (uHeightSurface * uWidthSurface * bpp * 2) / (xScale * yScale));

    pYBuff = pBuff;

    //YVU order in the buffer
    pVBuff = pYBuff + uWidthSurface * uHeightSurface * bpp;
    pUBuff = pVBuff + uWidthSurface * uHeightSurface * bpp / (xScale * yScale);

    pYUVBuff[0] = pYBuff;
    pYUVBuff[1] = pUBuff;
    pYUVBuff[2] = pVBuff;

    YUVPitch[0] = uWidthSurface * bpp;
    YUVPitch[1] = uWidthSurface * bpp / xScale;
    YUVPitch[2] = uWidthSurface * bpp / xScale;

    file = fopen(fileName, "rb");
    if(!file) {
        LOG_ERR("ReadYUVFrame: Error opening file: %s\n", fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    if(fseeko(file, uFrameNum * (off_t)uFrameSize, SEEK_SET)) {
        LOG_ERR("ReadYUVFrame: Error seeking file: %s\n", fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }
    //read Y U V separately
    for(i = 0; i < uHeight; i++) {
        if(fread(pYBuff, uWidth * bpp, 1, file) != 1) {
            LOG_ERR("ReadYUVFrame: Error reading file: %s\n", fileName);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        }
        pYBuff += uWidthSurface * bpp;
    }

    pChroma = bOrderUV ? pUBuff : pVBuff;
    for(i = 0; i < uHeight / yScale; i++) {
        if(fread(pChroma, uWidth * bpp / xScale, 1, file) != 1) {
            LOG_ERR("ReadYUVFrame: Error reading file: %s\n", fileName);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        }
        pChroma += uWidthSurface * bpp / xScale;
    }

    pChroma = bOrderUV ? pVBuff : pUBuff;
    for(i = 0; i < uHeight / yScale; i++) {
        if(fread(pChroma, uWidth * bpp / xScale, 1, file) != 1) {
            LOG_ERR("ReadYUVFrame: Error reading file: %s\n", fileName);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        }
        pChroma += uWidthSurface * bpp / xScale;
    }

    // nvmedia 10bit format [a7a600 0000 b7b6b5b4b3b2b1b0] b0-b7 occupies higher bits, a6 - a7 occupies lower bits
    // yuv 10bit format [b5b4b3b2b1b0a7a6 000000b7b6]
    // Example: nvmedia 10bit format 0x40 0x62 -> yuv 10bit 0x89 0x01
    if(pFrame->type == NvMediaSurfaceType_Video_420_10bit ||
       pFrame->type == NvMediaSurfaceType_Video_420_12bit ||
       pFrame->type == NvMediaSurfaceType_Video_422_10bit ||
       pFrame->type == NvMediaSurfaceType_Video_422_12bit ||
       pFrame->type == NvMediaSurfaceType_Video_444_10bit ||
       pFrame->type == NvMediaSurfaceType_Video_444_12bit) {
       // Y
       psrc = (NvU16*)(pYUVBuff[0]);
       size = uHeightSurface * uWidthSurface;
       for(i = 0; i < size; i++) {
            *(psrc + i) = (*(psrc + i)) << shift;
       }

       // U
       psrc = (NvU16*)(pYUVBuff[1]);
       size = uHeightSurface * uWidthSurface / (xScale * yScale);
       for(i = 0; i < size; i++) {
            *(psrc + i) = (*(psrc + i)) << shift;
       }

       // V
       psrc = (NvU16*)(pYUVBuff[2]);
       size = uHeightSurface * uWidthSurface / (xScale * yScale);
       for(i = 0; i < size; i++) {
            *(psrc + i) = (*(psrc + i)) << shift;
       }
    }

    NvMediaVideoSurfacePutBits(pFrame, NULL, (void **)pYUVBuff, YUVPitch);
    NvMediaVideoSurfaceUnlock(pFrame);

done:
    if(pBuff)
        free(pBuff);

    if(file)
        fclose(file);

    return ret;
}

NvMediaStatus
ReadRGBAFrame(
    char *fileName,
    NvU32 uFrameNum,
    NvU32 uWidth,
    NvU32 uHeight,
    NvMediaVideoSurface *pFrame)
{

    NvU8 *pBuff,*pRGBA;
    FILE *file;
    NvU32 uFrameSize = (uWidth * uHeight * 4);
    NvMediaVideoSurfaceMap surfaceMap;
    NvU32 uHeightSurface, uWidthSurface, uSurfaceSize;
    NvMediaStatus ret = NVMEDIA_STATUS_OK;
    NvU8 *pRGBABuff[3];
    NvU32 RGBAPitch[3];
    unsigned int i;

    NvMediaVideoSurfaceLock(pFrame, &surfaceMap);

    uHeightSurface = surfaceMap.lumaHeight;
    uWidthSurface  = surfaceMap.lumaWidth;
    /*Above doesn't work*/
    uHeightSurface = pFrame->height;
    uWidthSurface  = pFrame->width;
    uSurfaceSize = (uHeightSurface * uWidthSurface * 4) ;
    RGBAPitch[0] = uWidthSurface*4;

    if(!pFrame || uWidth > uWidthSurface || uHeight > uHeightSurface)
        return NVMEDIA_STATUS_BAD_PARAMETER;

    pBuff = (NvU8*)malloc(uSurfaceSize);
    if(!pBuff) {
        LOG_ERR("ReadRGBAFrame: Failed to allocate image buffer\n");
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }
    pRGBA = pBuff;

    memset(pBuff, 0x10, uSurfaceSize);

    pRGBABuff[0] = pRGBA;

    file = fopen(fileName, "rb");
    if(!file) {
        LOG_ERR("ReadRGBAFrame: Error opening file: %s\n", fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    if(fseek(file, uFrameNum * uFrameSize, SEEK_SET)) {
        LOG_ERR("ReadRGBAFrame: Error seeking file: %s\n", fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    //read RGBA
    for(i = 0; i < uHeight; i++) {
        if(fread(pRGBA, uWidth*4, 1, file) != 1) {
            LOG_ERR("ReadRGBAFrame: Error reading file: %s\n", fileName);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        }
        pRGBA += (uWidthSurface*4);
    }

    NvMediaVideoSurfacePutBits(pFrame, NULL, (void **)pRGBABuff, RGBAPitch);
    NvMediaVideoSurfaceUnlock(pFrame);

done:
    if(pBuff)
        free(pBuff);

    if(file)
        fclose(file);

    return ret;
}

static NvMediaStatus
ReadFrameNew(
    char *fileName,
    NvU32 uFrameNum,
    NvU32 uWidth,
    NvU32 uHeight,
    NvMediaVideoSurface *pFrame,
    NvMediaBool bOrderUV)
{
    NvU8 **pBuff = NULL;
    NvU32 *pBuffPitches = NULL;
    NvU32 frameSize = 0, surfaceSize = 0;
    NvU8 *buffer = NULL;
    NvU8 *pBuffer = NULL;
    NvU32 i, j, k, newk = 0;
    float *xScalePtr = NULL, *yScalePtr = NULL;
    unsigned int *bytePerPixelPtr = NULL;
    NvU32 numSurfaces = 1;
    unsigned int uHeightSurface, uWidthSurface;
    NvMediaVideoSurfaceMap surfaceMap;
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;
    FILE *file = NULL;
    unsigned int count, index;
    unsigned int surfType;
    NVM_SURF_FMT_DEFINE_ATTR(srcAttr);

    if(!fileName || !pFrame) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    file = fopen(fileName, "rb");
    if(!file) {
        LOG_ERR("%s: File open failed: %s\n", __func__, fileName);
        status = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    pBuff = (NvU8**)malloc(sizeof(NvU8*) * MAXM_NUM_SURFACES);
    if(!pBuff) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pBuffPitches = (NvU32*)calloc(1, sizeof(NvU32) * MAXM_NUM_SURFACES);
    if(!pBuffPitches) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    status = NvMediaSurfaceFormatGetAttrs(pFrame->type,
                                          srcAttr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaSurfaceFormatGetAttrs() failed\n", __func__);
        goto done;
    }

    surfType = srcAttr[NVM_SURF_ATTR_SURF_TYPE].value;
    if (surfType == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
        uHeightSurface = pFrame->height;
        uWidthSurface  = pFrame->width;
    } else {
        status = NvMediaVideoSurfaceLock(pFrame, &surfaceMap);
        if(status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s: NvMediaVideoSurfaceLock() failed\n", __func__);
            goto done;
        }

        NvMediaVideoSurfaceUnlock(pFrame);

        uHeightSurface = surfaceMap.lumaHeight;
        uWidthSurface  = surfaceMap.lumaWidth;
    }

    status = GetSurfParams(pFrame->type,
                           &xScalePtr,
                           &yScalePtr,
                           &bytePerPixelPtr,
                           &numSurfaces);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: GetSurfParams() failed\n", __func__);
        goto done;
    }

    surfaceSize = 0;
    frameSize = 0;
    for(i = 0; i < numSurfaces; i++) {
        surfaceSize += (uWidthSurface * xScalePtr[i] * uHeightSurface * yScalePtr[i] * bytePerPixelPtr[i]);
        frameSize += (uWidth * xScalePtr[i] * uHeight * yScalePtr[i] * bytePerPixelPtr[i]);
        pBuffPitches[i] = (NvU32)((float)uWidthSurface * xScalePtr[i]) * bytePerPixelPtr[i];
    }

    pBuffer = (NvU8*)calloc(1, surfaceSize);
    if(!pBuffer) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    buffer = pBuffer;
    memset(buffer,0x10,surfaceSize);
    for(i = 0; i < numSurfaces; i++) {
        pBuff[i] = buffer;
        if (i) {
            memset(pBuff[i], 0x80, (uHeightSurface * yScalePtr[i] * pBuffPitches[i]));
        }
        buffer = buffer + (NvU32)(uHeightSurface * yScalePtr[i] * pBuffPitches[i]);
    }

    if(uFrameNum > 0) {
        if(fseeko(file, uFrameNum * (off_t)frameSize, SEEK_SET)) {
            LOG_ERR("%s: Error seeking file\n", __func__);
            status = NVMEDIA_STATUS_ERROR;
            goto done;
        }
    }

    if((surfType == NvMediaSurfaceType_Image_RGBA) && strstr(fileName, ".png")) {
        LOG_ERR("%s: Use ReadPNGImage() function from common/nvmediatest_png.c\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
        goto done;
    }
    for(k = 0; k < numSurfaces; k++) {
        for(j = 0; j < uHeight*yScalePtr[k]; j++) {
            count = uWidth * xScalePtr[k] * bytePerPixelPtr[k];
            newk = (!bOrderUV && k ) ? (numSurfaces - k) : k;
            index = j * pBuffPitches[newk];
            if (fread(pBuff[newk] + index, count, 1, file) != 1) {
                LOG_ERR("ReadFrame: Error reading file: %s\n", fileName);
                status = NVMEDIA_STATUS_ERROR;
                goto done;
           }
        }
    }

    status = NvMediaVideoSurfaceLock(pFrame, &surfaceMap);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaVideoSurfaceLock() failed\n", __func__);
        goto done;
    }

    status = NvMediaVideoSurfacePutBits(pFrame, NULL, (void **)pBuff, pBuffPitches);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaVideoSurfacePutBits() failed\n", __func__);
    }
    NvMediaVideoSurfaceUnlock(pFrame);

done:
    if(file) {
        fclose(file);
    }

    if (pBuffer) {
        free(pBuffer);
    }

    if (pBuffPitches) {
        free(pBuffPitches);
    }

    if(pBuff) {
        free(pBuff);
    }

    return status;
}

NvMediaStatus
ReadFrame(
    char *fileName,
    NvU32 uFrameNum,
    NvU32 uWidth,
    NvU32 uHeight,
    NvMediaVideoSurface *pFrame,
    NvMediaBool bOrderUV)
{
    NvMediaStatus status;
    NVM_SURF_FMT_DEFINE_ATTR(srcAttr);

    status = NvMediaSurfaceFormatGetAttrs(pFrame->type,
                                          srcAttr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status == NVMEDIA_STATUS_NOT_SUPPORTED) {
        return ReadFrameOld(
                        fileName,
                        uFrameNum,
                        uWidth,
                        uHeight,
                        pFrame,
                        bOrderUV);
    } else if (status == NVMEDIA_STATUS_OK) {
        return ReadFrameNew(
                        fileName,
                        uFrameNum,
                        uWidth,
                        uHeight,
                        pFrame,
                        bOrderUV);
    } else {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return status;
    }
}

NvMediaStatus
CreateMemRGBASurf(
    unsigned int width,
    unsigned int height,
    NvMediaBool initflag,
    unsigned int initvalue,
    MemSurf **surf_out)
{
    MemSurf *surf;
    unsigned int *p;
    unsigned int pixels;

    surf = (MemSurf*)malloc(sizeof(MemSurf));
    if(!surf) {
        LOG_ERR("CreateMemRGBASurf: Cannot allocate surface structure\n");
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    memset(surf, 0, sizeof(MemSurf));
    surf->pSurf = (unsigned char*)malloc(width * height * 4);
    if(!surf->pSurf) {
        free(surf);
        LOG_ERR("CreateMemRGBASurf: Cannot allocate surface\n");
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }
    surf->width = width;
    surf->height = height;
    surf->pitch = width * 4;
    surf->bpp = 4;

    pixels = width * height;

    if(initflag) {
        p = (unsigned int *)(void *)surf->pSurf;
        while(pixels--) {
            *p++ = initvalue;
        }
    }
    *surf_out = surf;
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
DestroyMemSurf(
    MemSurf *surf)
{
    if(surf) {
        if(surf->pSurf)
            free(surf->pSurf);
        free(surf);
    }
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
DrawRGBARect(
    MemSurf *surf,
    NvMediaRect *rect,
    NvU8 R,
    NvU8 G,
    NvU8 B,
    NvU8 A)
{
    NvU32 color;
    NvU32 lines;
    NvU32 pixels;
    NvU8 *pPixelBase;

    if(!surf || !rect)
        return NVMEDIA_STATUS_BAD_PARAMETER;

    color = PACK_RGBA(R, G, B, A);
    lines = rect->y1 - rect->y0;
    pixels = rect->x1 - rect->x0;
    pPixelBase = surf->pSurf + rect->y0 * surf->pitch + rect->x0 * 4;

    while(lines--) {
        NvU32 i;
        NvU32 *pPixel = (NvU32 *)(void *)pPixelBase;
        for(i = 0; i < pixels; i++) {
            *pPixel++ = color;
        }

        pPixelBase += surf->pitch;
    }
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
PreMultiplyRGBASurf(
    MemSurf *surf)
{
    NvU32 pixels;
    NvU8 *p;
    NvU8 alpha;

    if(!surf)
        return NVMEDIA_STATUS_BAD_PARAMETER;

    pixels = surf->width * surf->height;
    p = (NvU8 *)(void *)surf->pSurf;

    while(pixels--) {
        alpha = *(p + 3);
        *p = ((NvU16)*p * (NvU16)alpha + 128) >> 8;
        p++;
        *p = ((NvU16)*p * (NvU16)alpha + 128) >> 8;
        p++;
        *p = ((NvU16)*p * (NvU16)alpha + 128) >> 8;
        p++;
        p++;
    }
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
CreateMemI8Surf(
    NvU32 width,
    NvU32 height,
    NvU8 init,
    MemSurf **surf_out)
{
    MemSurf *surf;

    surf = (MemSurf*)malloc(sizeof(MemSurf));
    if(!surf) {
        LOG_ERR("CreateMemI8Surf: Cannot allocate surface structure\n");
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    memset(surf, 0, sizeof(MemSurf));
    surf->pSurf = (unsigned char*)malloc(width * height);
    if(!surf->pSurf) {
        free(surf);
        LOG_ERR("CreateMemI8Surf: Cannot allocate surface\n");
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }
    surf->width = width;
    surf->height = height;
    surf->pitch = width;
    surf->bpp = 1;

    memset(surf->pSurf, init, width * height);

    *surf_out = surf;
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
DrawI8Rect(
    MemSurf *surf,
    NvMediaRect *rect,
    NvU8 index)
{
    NvU32 lines, pixels;
    NvU8 *pPixelBase;

    if(!surf || !rect)
        return NVMEDIA_STATUS_BAD_PARAMETER;

    lines = rect->y1 - rect->y0;
    pixels = rect->x1 - rect->x0;
    pPixelBase = surf->pSurf + rect->y0 * surf->pitch + rect->x0;

    while(lines--) {
        memset(pPixelBase, index, pixels);
        pPixelBase += surf->pitch;
    }
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
ReadRGBA(
    char *filename,
    unsigned int width,
    unsigned int height,
    MemSurf *rgbaSurface)
{
    FILE *file;

    if(!rgbaSurface || !rgbaSurface->pSurf)
        return NVMEDIA_STATUS_BAD_PARAMETER;

    file = fopen(filename, "rb");
    if(!file) {
        LOG_ERR("ReadRGBA: Failed opening file %s\n", filename);
        return NVMEDIA_STATUS_ERROR;
    }

    if(fread(rgbaSurface->pSurf, width * height * 4, 1, file) != 1) {
        LOG_ERR("ReadRGBA: Failed reading file %s\n", filename);
        return NVMEDIA_STATUS_ERROR;
    }

    fclose(file);

    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
WriteRGBA(
    char *filename,
    NvU32 outputBpp,
    NvU8 defaultAplha,
    MemSurf *rgbaSurface)
{
    NvU32 width = rgbaSurface->width;
    NvU32 height = rgbaSurface->height;
    NvMediaStatus ret = NVMEDIA_STATUS_ERROR;
    NvU8 *lineBuff = NULL;
    NvU32 i;
    FILE *f = fopen(filename, "wb");
    if(!f) {
        LOG_ERR("WriteRGBA: Cannot create file: %s", filename);
        goto WriteRGBA_end;
    }

    lineBuff = (NvU8*)malloc(width * outputBpp);
    if(!lineBuff) {
        LOG_ERR("WriteRGBA: Error allocating line buffer");
        goto WriteRGBA_end;
    }

    for(i = 0; i < height; i++) {
        NvU32 j;
        NvU8 *srcBuff = rgbaSurface->pSurf + i * rgbaSurface->pitch;
        NvU8 *dstBuff = lineBuff;

        for(j = 0; j < width; j++) {
            dstBuff[0] = srcBuff[0]; // R
            dstBuff[1] = srcBuff[1]; // G
            dstBuff[2] = srcBuff[2]; // B
            if(outputBpp == 4) {
                dstBuff[3] = rgbaSurface->bpp == 3 ? defaultAplha : srcBuff[3];
            }
            srcBuff += rgbaSurface->bpp;
            dstBuff += outputBpp;
        }
        if(fwrite(lineBuff, width * outputBpp, 1, f) != 1) {
            LOG_ERR("WriteRGBA: Error writing file: %s", filename);
            goto WriteRGBA_end;
        }
    }

    ret = NVMEDIA_STATUS_OK;
WriteRGBA_end:
    if(lineBuff)
        free(lineBuff);
    if(f)
        fclose(f);

    return ret;
}

NvMediaStatus
GetPPMFileDimensions(
    char *fileName,
    NvU16 *uWidth,
    NvU16 *uHeight)
{
    NvU32 uFileWidth;
    NvU32 uFileHeight;
    char buf[256], *t;
    FILE *file;

    file = fopen(fileName, "rb");
    if(!file) {
        LOG_ERR("GetPPMFileDimensions: Error opening file: %s\n", fileName);
        return NVMEDIA_STATUS_ERROR;
    }

    t = fgets(buf, 256, file);
    if(!t || strncmp(buf, "P6\n", 3)) {
        LOG_ERR("GetPPMFileDimensions: Invalid PPM header in file: %s\n", fileName);
        return NVMEDIA_STATUS_ERROR;
    }
    do {
        t = fgets(buf, 255, file);
        if(!t) {
            LOG_ERR("GetPPMFileDimensions: Invalid PPM header in file: %s\n", fileName);
            return NVMEDIA_STATUS_ERROR;
        }
    } while(!strncmp(buf, "#", 1));
    if(sscanf(buf, "%u %u", &uFileWidth, &uFileHeight) != 2) {
        LOG_ERR("GetPPMFileDimensions: Error getting PPM file resolution for the file: %s\n", fileName);
        return NVMEDIA_STATUS_ERROR;
    }
    if(uFileWidth < 16 || uFileWidth > 2048 ||
            uFileHeight < 16 || uFileHeight > 2048) {
        LOG_ERR("GetPPMFileDimensions: Invalid PPM file resolution: %ux%u in file: %s\n", uFileWidth, uFileHeight, fileName);
        return NVMEDIA_STATUS_ERROR;
    }

    fclose(file);

    *uWidth  = uFileWidth;
    *uHeight = uFileHeight;

    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
ReadPPMFrame(
    char *fileName,
    NvMediaVideoSurface *pFrame)
{
    NvU32 uSurfaceWidth;
    NvU32 uSurfaceHeight;
    NvU32 maxValue = 0;
    int num = 0;
    NvU32 x, y;
    char buf[256], *c;
    NvU8 *pBuff = NULL, *pRGBBuff;
    NvU32 uFrameSize;
    FILE *file;
    NvMediaVideoSurfaceMap surfaceMap;
    NvMediaStatus ret = NVMEDIA_STATUS_OK;

    if(!pFrame) {
        LOG_ERR("ReadPPMFrame: Failed allocating memory\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    file = fopen(fileName, "rb");
    if(!file) {
        LOG_ERR("ReadPPMFrame: Error opening file: %s\n", fileName);
        return NVMEDIA_STATUS_ERROR;
    }

    c = fgets(buf, 256, file);
    if(!c || strncmp(buf, "P6\n", 3)) {
        LOG_ERR("ReadPPMFrame: Invalid PPM header in file: %s\n", fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }
    do {
        c = fgets(buf, 255, file);
        if(!c) {
            LOG_ERR("ReadPPMFrame: Invalid PPM header in file: %s\n", fileName);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        }
    } while(!strncmp(buf, "#", 1));

    num = sscanf(buf, "%u %u %u", &uSurfaceWidth, &uSurfaceHeight, &maxValue);
    switch(num) {
    case 2:
        c = fgets(buf, 255, file);
        if(!c || strncmp(buf, "255\n", 4)) {
            LOG_ERR("ReadPPMFrame: Invalid PPM header in file: %s\n", fileName);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        } else
            break;
    case 3:
        if(maxValue != 255) {
            LOG_ERR("ReadPPMFrame: Invalid PPM header in file: %s\n", fileName);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        } else
            break;
    default:
        LOG_ERR("ReadPPMFrame: Error getting PPM file resolution in file: %s\n", fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    if(uSurfaceWidth < 16 || uSurfaceWidth > 2048 ||
            uSurfaceHeight < 16 || uSurfaceHeight > 2048) {
        LOG_ERR("ReadPPMFrame: Invalid PPM file resolution: %ux%u for file: %s\n", uSurfaceWidth, uSurfaceHeight, fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    uFrameSize = uSurfaceWidth * uSurfaceHeight * 3;
    pBuff = (NvU8*)malloc(uFrameSize);
    if(!pBuff) {
        LOG_ERR("ReadPPMFrame: Out of memory\n");
        ret = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }
    pRGBBuff = pBuff;

    if(fread(pRGBBuff, uFrameSize, 1, file) != 1) {
        LOG_ERR("ReadPPMFrame: Error reading file: %s\n", fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    NvMediaVideoSurfaceLock(pFrame, &surfaceMap);

    for(y = 0; y < uSurfaceHeight; y++) {
        NvU8 *pPixel = (NvU8 *)surfaceMap.pRGBA + surfaceMap.pitchRGBA * y;
        for(x = 0; x < uSurfaceWidth; x++) {
            *pPixel++ = *pRGBBuff++; // R
            *pPixel++ = *pRGBBuff++; // G
            *pPixel++ = *pRGBBuff++; // B
            *pPixel++ = 255;         // Alpha
        }
    }

done:
    NvMediaVideoSurfaceUnlock(pFrame);
    if(pBuff) free(pBuff);
    if(file) fclose(file);

    return ret;
}

NvMediaStatus
ReadPPM(
    char *fileName,
    NvU8 defaultAplha,
    MemSurf *rgbaSurface)
{
    NvU32 surfaceWidth;
    NvU32 surfaceHeight;
    NvU32 data = 0;
    char buf[256], *t;
    NvU8 *lineBuff = NULL;
    NvU32 i;
    NvMediaStatus ret = NVMEDIA_STATUS_ERROR;

    LOG_DBG("ReadPPM: Start - File: %s", fileName);

    FILE *f = fopen(fileName, "rb");
    if(!f) {
        LOG_ERR("ReadPPM: Error opening file: %s", fileName);
        goto ReadPPM_end;
    }

    t = fgets(buf, 256, f);
    if(!t || strncmp(buf, "P6\n", 3)) {
        LOG_ERR("ReadPPM: Invalid PPM header: %s", fileName);
        goto ReadPPM_end;
    }
    do {
        t = fgets(buf, 255, f);
        if(!t) {
            LOG_ERR("ReadPPM: Invalid PPM header: %s", fileName);
            goto ReadPPM_end;
        }
    } while(!strncmp(buf, "#", 1));
    if(sscanf(buf, "%u %u %u\n", &surfaceWidth, &surfaceHeight, &data) != 3) {
        LOG_ERR("ReadPPM: Error getting PPM file resolution - file: %s string: %s", fileName, buf);
        goto ReadPPM_end;
    }
    if(data != 255) {
        LOG_ERR("ReadPPM: Invalid PPM header (data: %u) resolution: %dx%d string: %s", data, surfaceWidth, surfaceHeight, buf);
        goto ReadPPM_end;
    }

    lineBuff = (NvU8*)malloc(surfaceWidth * 3);
    if(!lineBuff) {
        LOG_ERR("ReadPPM: Error allocating line buffer");
        goto ReadPPM_end;
    }

    for(i = 0; i < surfaceHeight; i++) {
        NvU32 j;
        NvU8 *srcBuff = lineBuff;
        NvU8 *dstBuff = rgbaSurface->pSurf + i * rgbaSurface->pitch;

        if(fread(lineBuff, surfaceWidth * 3, 1, f) != 1) {
            LOG_ERR("ReadPPM: Error reading file: %s", fileName);
            goto ReadPPM_end;
        }
        for(j = 0; j < surfaceWidth; j++) {
            dstBuff[0] = srcBuff[0]; // R
            dstBuff[1] = srcBuff[1]; // G
            dstBuff[2] = srcBuff[2]; // B
            if(rgbaSurface->bpp == 4)
                dstBuff[3] = defaultAplha;
            srcBuff += 3;
            dstBuff += rgbaSurface->bpp;
        }
    }

    ret = NVMEDIA_STATUS_OK;
ReadPPM_end:
    if(lineBuff)
        free(lineBuff);
    if(f)
        fclose(f);

    LOG_DBG("ReadPPM: End");

    return ret;
}

NvMediaStatus
WritePPM(
    char *fileName,
    MemSurf *rgbaSurface)
{
    NvU8 *lineBuff = NULL;
    NvU32 i;
    char header[256];
    NvMediaStatus ret = NVMEDIA_STATUS_ERROR;

    LOG_DBG("WritePPM: Start - File: %s", fileName);

    FILE *f = fopen(fileName, "wb");
    if(!f) {
        LOG_ERR("WritePPM: Error opening file: %s", fileName);
        goto WritePPM_end;
    }

    sprintf(header, "P6\n# NVIDIA\n%u %u %u\n", rgbaSurface->width, rgbaSurface->height, 255);

    if(fwrite(header, strlen(header), 1, f) != 1) {
        LOG_ERR("WritePPM: Error writing PPM file header: %s", fileName);
        goto WritePPM_end;
    }

    lineBuff = (NvU8*)malloc(rgbaSurface->width * 3);
    if(!lineBuff) {
        LOG_ERR("WritePPM: Error allocating line buffer");
        goto WritePPM_end;
    }

    for(i = 0; i < rgbaSurface->height; i++) {
        NvU32 j;
        NvU8 *srcBuff = rgbaSurface->pSurf + i * rgbaSurface->pitch;
        NvU8 *dstBuff = lineBuff;

        for(j = 0; j < rgbaSurface->width; j++) {
            dstBuff[0] = srcBuff[0]; // R
            dstBuff[1] = srcBuff[1]; // G
            dstBuff[2] = srcBuff[2]; // B
            srcBuff += rgbaSurface->bpp;
            dstBuff += 3;
        }
        if(fwrite(lineBuff, rgbaSurface->width * 3, 1, f) != 1) {
            LOG_ERR("WritePPM: Error writing file: %s", fileName);
            goto WritePPM_end;
        }
    }

    ret = NVMEDIA_STATUS_OK;
WritePPM_end:
    if(lineBuff)
        free(lineBuff);
    if(f)
        fclose(f);

    LOG_DBG("WritePPM: End");

    return ret;
}

NvMediaStatus
ReadPAL(
    char *filename,
    NvU32 *palette)
{
    NvMediaStatus ret = NVMEDIA_STATUS_ERROR;

    FILE *f = fopen(filename, "rb");
    if(!f) {
        LOG_ERR("ReadPAL: File: %s does not exist", filename);
        goto ReadPAL_end;
    }

    if(fread(palette, 256 * 4, 1, f) != 1) {
        LOG_ERR("ReadPAL: Error reading file: %s", filename);
        goto ReadPAL_end;
    }

    ret = NVMEDIA_STATUS_OK;
ReadPAL_end:
    if(f)
        fclose(f);

    return ret;
}

NvMediaStatus
ReadI8(
    char *filename,
    MemSurf *dstSurface)
{
    NvMediaStatus ret = NVMEDIA_STATUS_ERROR;
    NvU32 i;
    NvU8 *dst;

    FILE *f = fopen(filename, "rb");
    if(!f) {
        LOG_ERR("ReadI8: File: %s does not exist", filename);
        goto ReadI8_end;
    }

    dst = dstSurface->pSurf;
    for(i = 0; i < dstSurface->height; i++) {
        if(fread(dst, dstSurface->width, 1, f) != 1) {
            LOG_ERR("ReadI8: Error reading file: %s", filename);
            goto ReadI8_end;
        }
        dst += dstSurface->pitch;
    }

    ret = NVMEDIA_STATUS_OK;
ReadI8_end:
    if(f)
        fclose(f);

    return ret;
}

NvMediaStatus
static GetSurfaceCrcOld(
    NvMediaVideoSurface *surf,
    NvU32 width,
    NvU32 height,
    NvMediaBool monochromeFlag,
    NvU32 *crcOut)
{
    NvMediaVideoSurfaceMap surfMap;
    NvU32 lines, crc = 0;
    NvU8 *pBuff = NULL, *pDstBuff[3] = {NULL};
    NvU32 dstPitches[3];
    NvU8 *pY, *pV, *pU;
    NvU32 *pitchY, *pitchV, *pitchU;
    NvU32 bytePerPixel = 1;
    NvMediaStatus status;

    if(!surf || !crcOut) {
        LOG_ERR("GetSurfaceCrc: Bad parameter\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    if(NvMediaVideoSurfaceLock(surf, &surfMap) != NVMEDIA_STATUS_OK) {
        LOG_ERR("CheckSurfaceCrc: NvMediaVideoSurfaceLock failed\n");
        return NVMEDIA_STATUS_ERROR;
    }

    switch(surf->type) {
        case NvMediaSurfaceType_Video_420_10bit:
        case NvMediaSurfaceType_Video_420_12bit:
        case NvMediaSurfaceType_Video_422_10bit:
        case NvMediaSurfaceType_Video_422_12bit:
        case NvMediaSurfaceType_Video_444_10bit:
        case NvMediaSurfaceType_Video_444_12bit:
            bytePerPixel = 2;
            break;
        default:
            bytePerPixel = 1;
            break;
    }

    pBuff = (NvU8*)malloc(width * height * bytePerPixel * 4);
    if(!pBuff) {
        LOG_ERR("CheckSurfaceCrc: Failed to allocate frame buffer\n");
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    pDstBuff[0] = pBuff;
    pY = pDstBuff[0];
    pV = pDstBuff[1];
    pU = pDstBuff[2];
    pitchY = &dstPitches[0];
    pitchV = &dstPitches[1];
    pitchU = &dstPitches[2];

    switch(surf->type) {
        case NvMediaSurfaceType_YV24:
        case NvMediaSurfaceType_Video_444_10bit:
        case NvMediaSurfaceType_Video_444_12bit:
            pU = pY + width * height * bytePerPixel;
            pV = pU + width * height * bytePerPixel;
            *pitchY = *pitchV = *pitchU = width * bytePerPixel;
            break;
        case NvMediaSurfaceType_YV16:
        case NvMediaSurfaceType_YV16x2:
        case NvMediaSurfaceType_Video_422_10bit:
        case NvMediaSurfaceType_Video_422_12bit:
            pU = pY + width * height * bytePerPixel;
            pV = pU + width * height * bytePerPixel / 2;
            *pitchY = width * bytePerPixel;
            *pitchV = *pitchU = width * bytePerPixel / 2;
            break;
        case NvMediaSurfaceType_YV12:
        case NvMediaSurfaceType_Video_420_10bit:
        case NvMediaSurfaceType_Video_420_12bit:
            pU = pY + width * height * bytePerPixel;
            pV = pU + width * height * bytePerPixel / 4;
            *pitchY = width * bytePerPixel;
            *pitchV = *pitchU = width * bytePerPixel / 2;
            break;
        case NvMediaSurfaceType_R8G8B8A8:
        case NvMediaSurfaceType_R8G8B8A8_BottomOrigin:
            dstPitches[0] = width * 4;
            break;
        default:
            LOG_ERR("CheckSurfaceCrc: Invalid video surface type\n");
            free(pBuff);
            return NVMEDIA_STATUS_ERROR;
    }

    status = NvMediaVideoSurfaceGetBits(surf, NULL, (void **)pDstBuff, &dstPitches[0]);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("CheckSurfaceCrc: NvMediaVideoSurfaceGetBits failed\n");
        free(pBuff);
        return NVMEDIA_STATUS_ERROR;
    }
    NvMediaVideoSurfaceUnlock(surf);

    switch(surf->type) {
        case NvMediaSurfaceType_YV24:
        case NvMediaSurfaceType_YV16:
        case NvMediaSurfaceType_YV16x2:
        case NvMediaSurfaceType_YV12:
        case NvMediaSurfaceType_Video_420_10bit:
        case NvMediaSurfaceType_Video_420_12bit:
        case NvMediaSurfaceType_Video_422_10bit:
        case NvMediaSurfaceType_Video_422_12bit:
        case NvMediaSurfaceType_Video_444_10bit:
        case NvMediaSurfaceType_Video_444_12bit:
            lines = height;
            while(lines--) {
                crc = CalculateBufferCRC(*pitchY, crc, pY);
                pY += *pitchY;
            }

            if (monochromeFlag != NV_TRUE) {
                lines = height / 2;
                while(lines--) {
                    crc = CalculateBufferCRC(*pitchV, crc, pV);
                    pV += *pitchV;
                }
                lines = height / 2;
                while(lines--) {
                    crc = CalculateBufferCRC(*pitchU, crc, pU);
                    pU += *pitchU;
                }
            }
            break;
        case NvMediaSurfaceType_R8G8B8A8:
        case NvMediaSurfaceType_R8G8B8A8_BottomOrigin:
            lines = height;
            while(lines--) {
                crc = CalculateBufferCRC(dstPitches[0], crc, pDstBuff[0]);
                pDstBuff[0] += dstPitches[0];
            }
            break;
        default:
            LOG_ERR("CheckSurfaceCrc: Invalid video surface type\n");
            free(pBuff);
            return NVMEDIA_STATUS_ERROR;
    }

    *crcOut = crc;

    free(pBuff);
    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
static GetSurfaceCrcNew(
    NvMediaVideoSurface *videoSurface,
    NvU32 width,
    NvU32 height,
    NvMediaBool monochromeFlag,
    NvU32 *crcOut)
{
    NvMediaVideoSurfaceMap surfMap;
    NvU32 lines, crc = 0;
    NvMediaStatus status;
    NvU8 **pBuff = NULL;
    NvU32 *pBuffPitches = NULL;
    NvU32 *pBuffWidthInBytes = NULL;
    NvU8 *bufferSurf = NULL;
    NvU8 *bufferTmp = NULL;
    NvU32 i = 0;
    float *xScalePtr = NULL, *yScalePtr = NULL;
    unsigned int *bytePerPixelPtr = NULL;
    NvU32 numSurfaces = 1;
    NvU32 uHeightSurface, uWidthSurface, imageSize;

    if(!videoSurface || !crcOut) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    status = NvMediaVideoSurfaceLock(videoSurface, &surfMap);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaVideoSurfaceLock() failed\n", __func__);
        return status;
    }
    NvMediaVideoSurfaceUnlock(videoSurface);

    uHeightSurface = surfMap.lumaHeight;
    uWidthSurface  = surfMap.lumaWidth;

    if(width > uWidthSurface || height > uHeightSurface) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    status = GetSurfParams(videoSurface->type,
                           &xScalePtr,
                           &yScalePtr,
                           &bytePerPixelPtr,
                           &numSurfaces);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: GetSurfParams() failed\n", __func__);
        return status;
    }

    pBuff = (NvU8**)calloc(1,sizeof(NvU8*)*MAXM_NUM_SURFACES);
    if(!pBuff) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pBuffPitches = (NvU32*)calloc(1,sizeof(NvU32) * MAXM_NUM_SURFACES);
    if(!pBuffPitches) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pBuffWidthInBytes = (NvU32*)calloc(1,sizeof(NvU32) * MAXM_NUM_SURFACES);
    if(!pBuffWidthInBytes) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    imageSize = 0;
    for(i = 0; i < numSurfaces; i++) {
        imageSize += (uWidthSurface * xScalePtr[i] * uHeightSurface * yScalePtr[i] * bytePerPixelPtr[i]);
        pBuffWidthInBytes[i] = (NvU32)((float)width * xScalePtr[i]) * bytePerPixelPtr[i];
        pBuffPitches[i] = (NvU32)((float)uWidthSurface * xScalePtr[i]) * bytePerPixelPtr[i];
    }

    bufferSurf = (NvU8*)calloc(1, imageSize);
    if(!bufferSurf) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    memset(bufferSurf,0xFF,imageSize);
    bufferTmp = bufferSurf;
    for(i = 0; i < numSurfaces; i++) {
        pBuff[i] = bufferTmp;
        bufferTmp = bufferTmp + (NvU32)(uHeightSurface * yScalePtr[i] * pBuffPitches[i]);
    }

    status = NvMediaVideoSurfaceLock(videoSurface, &surfMap);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaVideoSurfaceLock() failed\n", __func__);
        goto done;
    }
    status = NvMediaVideoSurfaceGetBits(videoSurface, NULL, (void **)pBuff, pBuffPitches);
    NvMediaVideoSurfaceUnlock(videoSurface);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaVideoSurfaceGetBits failed\n", __func__);
        goto done;
    }
    NvMediaVideoSurfaceUnlock(videoSurface);

    for(i = 0; i < numSurfaces; i++) {
        if(i > 0 && monochromeFlag == NV_TRUE) {
            break;
        }
        lines = height*yScalePtr[i];
        bufferTmp = pBuff[i];
        while(lines--) {
            crc = CalculateBufferCRC(pBuffWidthInBytes[i], crc, bufferTmp);
            bufferTmp += pBuffPitches[i];
        }
    }

    *crcOut = crc;

done:
    if(pBuff) {
        free(pBuff);
    }

    if(pBuffPitches) {
        free(pBuffPitches);
    }

    if(pBuffWidthInBytes) {
        free(pBuffWidthInBytes);
    }

    if(bufferSurf) {
        free(bufferSurf);
    }

    return status;
}

NvMediaStatus
GetSurfaceCrc(
    NvMediaVideoSurface *surf,
    NvU32 width,
    NvU32 height,
    NvMediaBool monochromeFlag,
    NvU32 *crcOut)
{
    NvMediaStatus status;
    NVM_SURF_FMT_DEFINE_ATTR(srcAttr);

    if(!surf || !crcOut) {
        LOG_ERR("GetImageCrc: Bad parameter\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    status = NvMediaSurfaceFormatGetAttrs(surf->type,
                                          srcAttr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status == NVMEDIA_STATUS_NOT_SUPPORTED) {
        return GetSurfaceCrcOld(
                                surf,
                                width,
                                height,
                                monochromeFlag,
                                crcOut);
    } else if (status == NVMEDIA_STATUS_OK) {
        return GetSurfaceCrcNew(
                                surf,
                                width,
                                height,
                                monochromeFlag,
                                crcOut);
    } else {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return status;
    }
}

NvMediaStatus
CheckSurfaceCrc(
    NvMediaVideoSurface *surf,
    NvU32 width,
    NvU32 height,
    NvMediaBool monochromeFlag,
    NvU32 ref,
    NvMediaBool *isMatching)
{
    NvU32 crc = 0;
    NvMediaStatus status = NVMEDIA_STATUS_OK;

    status = GetSurfaceCrc(surf, width, height, monochromeFlag, &crc);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("CheckSurfaceCrc: GetSurfaceCrc failed\n");
        return NVMEDIA_STATUS_ERROR;
    }

    if(crc != ref) {
        LOG_WARN("CheckSurfaceCrc: Encountered CRC mismatch.\n");
        LOG_WARN("CheckSurfaceCrc: Calculated CRC: %8x (%d). Expected CRC: %8x (%d).\n", crc, crc, ref, ref);
        *isMatching = NVMEDIA_FALSE;
    } else {
        *isMatching = NVMEDIA_TRUE;
    }

    return NVMEDIA_STATUS_OK;
}

static NvMediaStatus
GetImageCrcOld(
    NvMediaImage *image,
    NvU32 width,
    NvU32 height,
    NvU32 *crcOut,
    NvU32 rawBytesPerPixel)
{
    NvMediaImageSurfaceMap surfaceMap;
    NvU32 lines, crc = 0;
    NvU8 *pBuff = NULL, *pYUVBuff[3] = {NULL};
    NvU32 YUVPitch[3];
    NvU8 *pYBuff = NULL, *pUBuff = NULL, *pVBuff = NULL;
    NvU32 uHeightSurface, uWidthSurface, imageSize;
    NvU32 xScale = 1, yScale = 1;
    NvMediaStatus status;

    if(!image || !crcOut) {
        LOG_ERR("GetImageCrc: Bad parameter\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    if(NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap) != NVMEDIA_STATUS_OK) {
        LOG_ERR("GetImageCrc: NvMediaImageLock failed\n");
        return NVMEDIA_STATUS_ERROR;
    }

    uHeightSurface = surfaceMap.height;
    uWidthSurface  = surfaceMap.width;

    if(width > uWidthSurface || height > uHeightSurface) {
        LOG_ERR("GetImageCrc: Bad parameters\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    pBuff = (NvU8* )malloc(uHeightSurface * uWidthSurface * 4);
    if(!pBuff) {
        LOG_ERR("GetImageCrc: Failed to allocate frame buffer\n");
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    switch(image->type) {
        case NvMediaSurfaceType_Image_YUV_444:
            YUVPitch[0] = uWidthSurface;
            imageSize = width * height * 3;
            memset(pBuff, 0x10, uWidthSurface * uHeightSurface);
            memset(pBuff + uWidthSurface * uHeightSurface, 0x80, (uHeightSurface * uWidthSurface) * 2);
            //YVU order in the buffer
            xScale = 1;
            yScale = 1;
            break;
        case NvMediaSurfaceType_Image_YUV_422:
        case NvMediaSurfaceType_Image_YUYV_422:
            YUVPitch[0] = uWidthSurface;
            imageSize = 2 * (width * height);
            memset(pBuff, 0x10, uWidthSurface * uHeightSurface);
            memset(pBuff + uWidthSurface * uHeightSurface, 0x80, (uHeightSurface * uWidthSurface));
            xScale = 2;
            yScale = 1;
            break;
        case NvMediaSurfaceType_Image_YUV_420:
            YUVPitch[0] = uWidthSurface;
            imageSize = (width * height * 3) / 2;
            memset(pBuff, 0x10, uWidthSurface * uHeightSurface);
            memset(pBuff + uWidthSurface * uHeightSurface, 0x80, (uHeightSurface * uWidthSurface) / 2);
            xScale = 2;
            yScale = 2;
            break;
        case NvMediaSurfaceType_Image_RGBA:
            imageSize = uWidthSurface * uHeightSurface * 4;
            memset(pBuff, 0x10, imageSize);
            YUVPitch[0] = width * 4;
            break;
        case NvMediaSurfaceType_Image_RAW:
            imageSize = uWidthSurface * uHeightSurface * rawBytesPerPixel;
            memset(pBuff, 0x10, imageSize);
            YUVPitch[0] = width * rawBytesPerPixel;
            break;
        default:
            LOG_ERR("GetImageCrc: Invalid image surface type, not supported\n");
            free(pBuff);
            return NVMEDIA_STATUS_ERROR;
    }

    YUVPitch[1] = uWidthSurface / xScale;
    YUVPitch[2] = uWidthSurface / xScale;

    pYBuff = pBuff;
    pUBuff = pYBuff + uWidthSurface * uHeightSurface;
    pVBuff = pUBuff + (uWidthSurface * uHeightSurface) / (xScale * yScale);

    pYUVBuff[0] = pYBuff;
    pYUVBuff[1] = pUBuff;
    pYUVBuff[2] = pVBuff;

    status = NvMediaImageGetBits(image, NULL, (void **)pYUVBuff, &YUVPitch[0]);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("GetImageCrc: NvMediaVideoSurfaceGetBits failed\n");
        free(pBuff);
        return NVMEDIA_STATUS_ERROR;
    }
    NvMediaImageUnlock(image);
    switch(image->type) {
        case NvMediaSurfaceType_Image_YUV_444:
        case NvMediaSurfaceType_Image_YUV_422:
        case NvMediaSurfaceType_Image_YUYV_422:
        case NvMediaSurfaceType_Image_YUV_420:
            lines = height;
            while(lines--) {
                crc = CalculateBufferCRC(YUVPitch[0], crc, pYBuff);
                pYBuff += YUVPitch[0];
            }

            lines = height / 2;
            while(lines--) {
                crc = CalculateBufferCRC(YUVPitch[1], crc, pVBuff);
                pVBuff += YUVPitch[1];
            }
            lines = height / 2;
            while(lines--) {
               crc = CalculateBufferCRC(YUVPitch[2], crc, pUBuff);
               pUBuff += YUVPitch[2];
            }
            break;
        case NvMediaSurfaceType_Image_RGBA:
            lines = height;
            while(lines--) {
                crc = CalculateBufferCRC(YUVPitch[0], crc, pYUVBuff[0]);
                pYUVBuff[0] += YUVPitch[0];
            }
            break;
        case NvMediaSurfaceType_Image_RAW:
            lines = height + ((image->embeddedDataTopSize + image->embeddedDataBottomSize)/(width * rawBytesPerPixel));
            while(lines--) {
                crc = CalculateBufferCRC(YUVPitch[0], crc, pYUVBuff[0]);
                pYUVBuff[0] += YUVPitch[0];
            }
            break;
        default:
            LOG_ERR("GetImageCrc: Invalid image type, not supported\n");
            free(pBuff);
            return NVMEDIA_STATUS_ERROR;
    }

    *crcOut = crc;
    free(pBuff);
    return NVMEDIA_STATUS_OK;
}

static NvMediaStatus
GetImageCrcNew(
    NvMediaImage *image,
    NvU32 width,
    NvU32 height,
    NvU32 *crcOut,
    NvU32 rawBytesPerPixel)
{
    NvMediaImageSurfaceMap surfaceMap;
    NvU32 lines, crc = 0;
    NvU32 uHeightSurface, uWidthSurface, imageSize;
    NvMediaStatus status;
    NvU8 **pBuff = NULL;
    NvU32 *pBuffPitches = NULL;
    NvU32 *pBuffWidthInBytes = NULL;
    NvU8 *bufferImg = NULL;
    NvU8 *bufferTmp = NULL;
    NvU32 i = 0;
    float *xScalePtr = NULL, *yScalePtr = NULL;
    unsigned int *bytePerPixelPtr = NULL;
    NvU32 numSurfaces = 1;
    NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);

    if(!image || !crcOut) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    status = NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageLock() failed\n", __func__);
        return status;
    }
    NvMediaImageUnlock(image);

    uHeightSurface = surfaceMap.height;
    uWidthSurface  = surfaceMap.width;

    status = NvMediaSurfaceFormatGetAttrs(image->type,surfFormatAttrs,NVM_SURF_FMT_ATTR_MAX);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaSurfaceFormatGetAttrs() failed\n",__func__);
        return status;
    }
    for(i=0; i < NVM_SURF_FMT_ATTR_MAX; i++){
        if(surfFormatAttrs[i].type == NVM_SURF_ATTR_SURF_TYPE){
            if(surfFormatAttrs[i].value == NVM_SURF_ATTR_SURF_TYPE_RAW) {
                uHeightSurface = uHeightSurface +
                                 ((image->embeddedDataTopSize + image->embeddedDataBottomSize)/
                                  (width * rawBytesPerPixel));
                break;
            }
        }
    }

    if(width > uWidthSurface || height > uHeightSurface) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    pBuff = (NvU8** )calloc(1,sizeof(NvU8*)*MAXM_NUM_SURFACES);
    if(!pBuff) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pBuffPitches = (NvU32*)calloc(1,sizeof(NvU32) * MAXM_NUM_SURFACES);
    if(!pBuffPitches) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pBuffWidthInBytes = (NvU32*)calloc(1,sizeof(NvU32) * MAXM_NUM_SURFACES);
    if(!pBuffWidthInBytes) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    status = GetSurfParams(image->type,
                           &xScalePtr,
                           &yScalePtr,
                           &bytePerPixelPtr,
                           &numSurfaces);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: GetSurfParams() failed\n", __func__);
        goto done;
    }

    imageSize = 0;
    for(i = 0; i < numSurfaces; i++) {
        imageSize += (uWidthSurface * xScalePtr[i] * uHeightSurface * yScalePtr[i] * bytePerPixelPtr[i]);
        pBuffWidthInBytes[i] = (NvU32)((float)width * xScalePtr[i]) * bytePerPixelPtr[i];
        pBuffPitches[i] = (NvU32)((float)uWidthSurface * xScalePtr[i]) * bytePerPixelPtr[i];
    }

    bufferImg = (NvU8*)calloc(1, imageSize);
    if(!bufferImg) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    memset(bufferImg,0xFF,imageSize);
    bufferTmp = bufferImg;
    for(i = 0; i < numSurfaces; i++) {
        pBuff[i] = bufferTmp;
        bufferTmp = bufferTmp + (NvU32)(uHeightSurface * yScalePtr[i] * pBuffPitches[i]);
    }

    status = NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageLock() failed\n", __func__);
        goto done;
    }

    status = NvMediaImageGetBits(image, NULL, (void **)pBuff, pBuffPitches);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageGetBits failed\n", __func__);
        goto done;
    }
    NvMediaImageUnlock(image);

    height = height + ((image->embeddedDataTopSize + image->embeddedDataBottomSize)/(width * rawBytesPerPixel));
    for(i = 0; i < numSurfaces; i++) {
        lines = height*yScalePtr[i];
        bufferTmp = pBuff[i];
        while(lines--) {
            crc = CalculateBufferCRC(pBuffWidthInBytes[i], crc, bufferTmp);
            bufferTmp += pBuffPitches[i];
        }
    }

    *crcOut = crc;

done:
    if(pBuff) {
        free(pBuff);
    }

    if(pBuffPitches) {
        free(pBuffPitches);
    }

    if(pBuffWidthInBytes) {
        free(pBuffWidthInBytes);
    }

    if(bufferImg) {
        free(bufferImg);
    }

    return status;
}

NvMediaStatus
GetImageCrc(
    NvMediaImage *image,
    NvU32 width,
    NvU32 height,
    NvU32 *crcOut,
    NvU32 rawBytesPerPixel)
{
    NvMediaStatus status;
    NVM_SURF_FMT_DEFINE_ATTR(srcAttr);

    if(!image || !crcOut) {
        LOG_ERR("GetImageCrc: Bad parameter\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    status = NvMediaSurfaceFormatGetAttrs(image->type,
                                          srcAttr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status == NVMEDIA_STATUS_NOT_SUPPORTED) {
        return GetImageCrcOld(
                            image,
                            width,
                            height,
                            crcOut,
                            rawBytesPerPixel);
    } else if (status == NVMEDIA_STATUS_OK) {
        return GetImageCrcNew(
                            image,
                            width,
                            height,
                            crcOut,
                            rawBytesPerPixel);
    } else {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return status;
    }
}

NvMediaStatus
CheckImageCrc(
    NvMediaImage *image,
    NvU32 width,
    NvU32 height,
    NvU32 ref,
    NvMediaBool *isMatching,
    NvU32 rawBytesPerPixel)
{
    NvU32 crc = 0;
    NvMediaStatus status = NVMEDIA_STATUS_OK;

    status = GetImageCrc(image, width, height, &crc, rawBytesPerPixel);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("CheckImageCrc: GetImageCrc failed\n");
        return NVMEDIA_STATUS_ERROR;
    }

    if(crc != ref) {
        LOG_WARN("CheckImageCrc: Encountered CRC mismatch.\n");
        LOG_WARN("CheckImageCrc: Calculated CRC: %8x (%d). Expected CRC: %8x (%d).\n", crc, crc, ref, ref);
        *isMatching = NVMEDIA_FALSE;
    } else {
        *isMatching = NVMEDIA_TRUE;
    }

    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
WriteRAWImageToRGBA(
    char *filename,
    NvMediaImage *image,
    NvMediaBool appendFlag,
    NvU32 bytesPerPixel)
{
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;
    NvMediaImageSurfaceMap surfaceMap;
    NvU32 width, height;
    FILE *file = NULL;
    NvU32 *rgbaBuffer;
    NvU32 i, j;
    NvU8 *rawBuffer;
    NvU16 *evenLine;
    NvU16 *oddLine;

    if(!image || !filename) {
        LOG_ERR("WriteRAWImageToRGBA: Bad parameter\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    if(NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap) != NVMEDIA_STATUS_OK) {
        LOG_ERR("WriteRAWImageToRGBA: NvMediaImageLock failed\n");
        return NVMEDIA_STATUS_ERROR;
    }

    height = surfaceMap.height;
    width  = surfaceMap.width;

    if(!(file = fopen(filename, appendFlag ? "ab" : "wb"))) {
        LOG_ERR("WriteRAWImageToRGBA: file open failed: %s\n", filename);
        perror(NULL);
        return NVMEDIA_STATUS_ERROR;
    }

    if(!(rgbaBuffer = (NvU32*)calloc(1, width * height * 4))) {
        LOG_ERR("WriteRAWImageToRGBA: Out of memory\n");
        fclose(file);
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    rawBuffer = (NvU8 *)surfaceMap.surface[0].mapping;

    for(j = 0; j < height - 2; j = j + 2) {
        evenLine = (NvU16 *)rawBuffer;
        oddLine = (NvU16 *)(rawBuffer + surfaceMap.surface[0].pitch);
        for(i = 0; i < width - 2; i += 2) {
            rgbaBuffer[i + j * width] = ((NvU32)(oddLine[i + 1] >> 6)) |              // R
                                         ((NvU32)(evenLine[i + 1] >> 6) << 8) |       // G
                                         ((NvU32)(evenLine[i] >> 6) << 16) |          // B
                                         0xFF000000;
            rgbaBuffer[i + j * width + 1] = ((NvU32)(oddLine[i + 1] >> 6)) |          // R
                                             ((NvU32)(evenLine[i + 1] >> 6) << 8) |   // G
                                             ((NvU32)(evenLine[i] >> 6) << 16) |      // B
                                             0xFF000000;
            rgbaBuffer[i + (j + 1) * width] = ((NvU32)(oddLine[i + 1] >> 6)) |         // R
                                               ((NvU32)(oddLine[i] >> 6) << 8) |       // G
                                               ((NvU32)(evenLine[i] >> 6) << 16) |     // B
                                               0xFF000000;
            rgbaBuffer[i + (j + 1) * width + 1] = ((NvU32)(oddLine[i + 1] >> 6)) |      // R
                                                   ((NvU32)(oddLine[i] >> 6) << 8) |    // G
                                                   ((NvU32)(evenLine[i] >> 6) << 16) |  // B
                                                   0xFF000000;
        }
        rawBuffer += surfaceMap.surface[0].pitch * 2;
    }

    if(fwrite(rgbaBuffer, width * height * 4, 1, file) != 1) {
        LOG_ERR("WriteRAWImageToRGBA: file write failed\n");
        goto done;
    }

    status = NVMEDIA_STATUS_OK;

done:
    fclose(file);

    NvMediaImageUnlock(image);

    if(rgbaBuffer)
        free(rgbaBuffer);

    return status;
}

static NvMediaStatus
WriteImageOld(
    char *filename,
    NvMediaImage *image,
    NvMediaBool uvOrderFlag,
    NvMediaBool appendFlag,
    NvU32 bytesPerPixel)
{
    NvMediaImageSurfaceMap surfaceMap;
    unsigned char *pBuff = NULL, *pDstBuff[3] = {NULL}, *pChroma;
    unsigned int dstPitches[3];
    unsigned int imageSize = 0, width, height;
    int indexU = 1, indexV = 2;
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;
    FILE *file = NULL;
    unsigned int xScale = 1;
    unsigned int yScale = 1;

    if(!image || !filename) {
        LOG_ERR("WriteImage: Bad parameter\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    if(NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap) != NVMEDIA_STATUS_OK) {
        LOG_ERR("WriteImage: NvMediaImageLock failed\n");
        return NVMEDIA_STATUS_ERROR;
    }

    height = surfaceMap.height;
    width  = surfaceMap.width;

    if(!(file = fopen(filename, appendFlag ? "ab" : "wb"))) {
        LOG_ERR("WriteImage: file open failed: %s\n", filename);
        perror(NULL);
        return NVMEDIA_STATUS_ERROR;
    }

    switch(image->type) {
        case NvMediaSurfaceType_Image_YUV_444:
            dstPitches[0] = width;
            xScale = 1;
            yScale = 1;
            imageSize = width * height * 3;
            break;
        case NvMediaSurfaceType_Image_YUV_422:
        case NvMediaSurfaceType_Image_YUYV_422:
            dstPitches[0] = width;
            xScale = 2;
            yScale = 1;
            imageSize = width * height * 2;
            break;
        case NvMediaSurfaceType_Image_YUV_420:
            dstPitches[0] = width;
            xScale = 2;
            yScale = 2;
            imageSize = width * height * 3 / 2;
            break;
        case NvMediaSurfaceType_Image_RGBA:
            dstPitches[0] = width * 4;
            imageSize = width * height * 4;
            break;
        case NvMediaSurfaceType_Image_RAW:
            dstPitches[0] = width * bytesPerPixel;
            imageSize = width * height * bytesPerPixel;
            break;
        case NvMediaSurfaceType_Image_V16Y16U16X16:
            dstPitches[0] = width * 8;
            imageSize = width * height * 8;
            break;
        case NvMediaSurfaceType_Image_X2U10Y10V10:
            dstPitches[0] = width * 4;
            imageSize = width * height * 4;
            break;
        case NvMediaSurfaceType_Image_Y16:
        case NvMediaSurfaceType_Image_Y10:
            dstPitches[0] = width * 2;
            xScale = 1;
            yScale = 1;
            imageSize = width * height * 2;
            break;
        case NvMediaSurfaceType_Image_Monochrome:
        default:
            LOG_ERR("WriteImage: Invalid video surface type\n");
            goto done;
    }

    imageSize += image->embeddedDataTopSize;
    imageSize += image->embeddedDataBottomSize;

    if(!(pBuff = (unsigned char*)malloc(imageSize))) {
        LOG_ERR("WriteImage: Out of memory\n");
        fclose(file);
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    pDstBuff[0] = pBuff;
    pDstBuff[indexV] = pDstBuff[0] + width * height;
    pDstBuff[indexU] = pDstBuff[indexV] + (width * height) / (xScale * yScale);

    dstPitches[indexU] = width / xScale;
    dstPitches[indexV] = width / xScale;

    status = NvMediaImageGetBits(image, NULL, (void **)pDstBuff, dstPitches);
    NvMediaImageUnlock(image);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("WriteImage: NvMediaVideoSurfaceGetBits() failed\n");
        goto done;
    }

    switch(image->type) {
        case NvMediaSurfaceType_Image_YUV_444:
        case NvMediaSurfaceType_Image_YUV_422:
        case NvMediaSurfaceType_Image_YUYV_422:
        case NvMediaSurfaceType_Image_YUV_420:
            if(fwrite(pDstBuff[0], width * height, 1, file) != 1) {
                LOG_ERR("WriteImage, line %d: file write failed\n", __LINE__);
                goto done;
            }
            pChroma = uvOrderFlag ? pDstBuff[indexU] : pDstBuff[indexV];
            if(fwrite(pChroma, (width * height) / (xScale * yScale), 1, file) != 1) {
                LOG_ERR("WriteImage, line %d: file write failed\n", __LINE__);
                goto done;
            }
            pChroma = uvOrderFlag ? pDstBuff[indexV] : pDstBuff[indexU];
            if(fwrite(pChroma, (width * height) / (xScale * yScale), 1, file) != 1) {
                LOG_ERR("WriteImage, line %d: file write failed\n", __LINE__);
                goto done;
            }
            break;
        case NvMediaSurfaceType_Image_V16Y16U16X16:
        case NvMediaSurfaceType_Image_X2U10Y10V10:
        case NvMediaSurfaceType_Image_Y16:
        case NvMediaSurfaceType_Image_Y10:
        case NvMediaSurfaceType_Image_RGBA:
        case NvMediaSurfaceType_Image_RAW:
            if(fwrite(pDstBuff[0], imageSize, 1, file) != 1) {
                LOG_ERR("WriteImage, line %d: file write failed\n", __LINE__);
                goto done;
            }
            break;
        default:
            LOG_ERR("WriteImage: Invalid image surface type\n");
            goto done;
    }

    status = NVMEDIA_STATUS_OK;

done:
    if(pBuff)
        free(pBuff);

    if(file)
        fclose(file);

    return status;
}

static NvMediaStatus
WriteImageNew(
    char *filename,
    NvMediaImage *image,
    NvMediaBool uvOrderFlag,
    NvMediaBool appendFlag,
    NvU32 bytesPerPixel)
{
    NvU8 **pBuff = NULL;
    NvU32 *pBuffPitches = NULL;
    NvU32 imageSize = 0;
    unsigned int size[3] ={0};
    NvU8 *buffer = NULL;
    NvU8 *pBuffer = NULL;
    NvU32 i, k, newk = 0;
    float *xScalePtr = NULL, *yScalePtr = NULL;
    unsigned int *bytePerPixelPtr = NULL;
    NvU32 numSurfaces = 1;
    unsigned int uHeightSurface, uWidthSurface;
    NvMediaImageSurfaceMap surfaceMap;
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;
    FILE *file = NULL;

    if(!image || !filename) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    status = NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageLock() failed\n", __func__);
        return status;
    }
    NvMediaImageUnlock(image);

    file = fopen(filename, appendFlag ? "ab" : "wb");
    if(!file) {
        LOG_ERR("%s: file open failed: %s\n", __func__, filename);
        return NVMEDIA_STATUS_ERROR;
    }

    pBuff = (NvU8**)malloc(sizeof(NvU8*)*MAXM_NUM_SURFACES);
    if(!pBuff) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pBuffPitches = (NvU32*)calloc(1,sizeof(NvU32) * MAXM_NUM_SURFACES);
    if(!pBuffPitches) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    uHeightSurface = surfaceMap.height;
    uWidthSurface  = surfaceMap.width;

    status = GetSurfParams(image->type,
                           &xScalePtr,
                           &yScalePtr,
                           &bytePerPixelPtr,
                           &numSurfaces);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: GetSurfParams() failed\n", __func__);
        goto done;
    }

    imageSize = 0;
    for(i = 0; i < numSurfaces; i++) {
        size[i] = (uWidthSurface * xScalePtr[i] * uHeightSurface * yScalePtr[i] * bytePerPixelPtr[i]);
        imageSize += size[i];
        pBuffPitches[i] = (NvU32)((float)uWidthSurface * xScalePtr[i]) * bytePerPixelPtr[i];
    }

    // Embedded data size needs to be included for RAW surftype
    size[0] += image->embeddedDataTopSize;
    size[0] += image->embeddedDataBottomSize;
    imageSize += image->embeddedDataTopSize;
    imageSize += image->embeddedDataBottomSize;

    buffer = (NvU8*)calloc(1, imageSize);
    if(!buffer) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pBuffer = buffer;
    memset(buffer,0xFF,imageSize);
    for(i = 0; i < numSurfaces; i++) {
        pBuff[i] = buffer;
        buffer = buffer + (NvU32)(uHeightSurface * yScalePtr[i] * pBuffPitches[i]);
    }

    status = NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageLock() failed\n", __func__);
        goto done;
    }
    status = NvMediaImageGetBits(image, NULL, (void **)pBuff, pBuffPitches);
    NvMediaImageUnlock(image);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaVideoSurfaceGetBits() failed\n", __func__);
        goto done;
    }

    for(k = 0; k < numSurfaces; k++) {
       newk = (!uvOrderFlag && k ) ? (numSurfaces - k) : k;
       if (fwrite(pBuff[newk],size[newk],1,file) != 1) {
           LOG_ERR("%s: File write failed\n", __func__);
           status = NVMEDIA_STATUS_ERROR;
           goto done;
       }
    }

done:
    if(file) {
        fclose(file);
    }

    if(pBuff) {
        free(pBuff);
    }

    if (pBuffPitches) {
        free(pBuffPitches);
    }

    if (pBuffer) {
        free(pBuffer);
    }

    return status;
}

NvMediaStatus
WriteImage(
    char *filename,
    NvMediaImage *image,
    NvMediaBool uvOrderFlag,
    NvMediaBool appendFlag,
    NvU32 bytesPerPixel)
{
    NvMediaStatus status;
    NVM_SURF_FMT_DEFINE_ATTR(srcAttr);

    if(!image || !filename) {
        LOG_ERR("WriteImage: Bad parameter\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    status = NvMediaSurfaceFormatGetAttrs(image->type,
                                          srcAttr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status == NVMEDIA_STATUS_NOT_SUPPORTED) {
        return WriteImageOld(filename,
                       image,
                       uvOrderFlag,
                       appendFlag,
                       bytesPerPixel);
    } else if (status == NVMEDIA_STATUS_OK) {
        return WriteImageNew(filename,
                       image,
                       uvOrderFlag,
                       appendFlag,
                       bytesPerPixel);
    } else {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return status;
    }
}

static NvMediaStatus
ReadImageOld(
    char *fileName,
    NvU32 frameNum,
    NvU32 width,
    NvU32 height,
    NvMediaImage *image,
    NvMediaBool uvOrderFlag,
    NvU32 bytesPerPixel)
{
    NvU32 xScale = 1, yScale = 1, imageSize = 0, i;
    NvU32 uHeightSurface, uWidthSurface, YUVPitch[3];
    NvU8 *pYUVBuff[3], *pBuff = NULL, *pYBuff, *pUBuff = NULL, *pVBuff = NULL, *pChroma;
    FILE *file = NULL;
    NvMediaImageSurfaceMap surfaceMap;
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;
    MemSurf *memSurface = NULL;

    if(NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap) != NVMEDIA_STATUS_OK) {
        LOG_ERR("ReadImage: NvMediaImageLock failed\n");
        goto done;
    }

    uHeightSurface = surfaceMap.height;
    uWidthSurface  = surfaceMap.width;

    if(!image || width > uWidthSurface || height > uHeightSurface) {
        LOG_ERR("ReadImage: Bad parameters\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    pBuff = (NvU8*)malloc(uHeightSurface * uWidthSurface * 4);
    if(!pBuff) {
        LOG_ERR("ReadImage: Failed to allocate image buffer\n");
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    switch(image->type) {
        case NvMediaSurfaceType_Image_YUV_444:
            YUVPitch[0] = uWidthSurface;
            imageSize = width * height * 3;
            memset(pBuff, 0x10, uWidthSurface * uHeightSurface);
            memset(pBuff + uWidthSurface * uHeightSurface, 0x80, (uHeightSurface * uWidthSurface) * 2);
            //YVU order in the buffer
            xScale = 1;
            yScale = 1;
            break;
        case NvMediaSurfaceType_Image_YUV_422:
        case NvMediaSurfaceType_Image_YUYV_422:
            YUVPitch[0] = uWidthSurface;
            imageSize = 2 * (width * height);
            memset(pBuff, 0x10, uWidthSurface * uHeightSurface);
            memset(pBuff + uWidthSurface * uHeightSurface, 0x80, (uHeightSurface * uWidthSurface));
            xScale = 2;
            yScale = 1;
            break;
        case NvMediaSurfaceType_Image_YUV_420:
            YUVPitch[0] = uWidthSurface;
            imageSize = (width * height * 3) / 2;
            memset(pBuff, 0x10, uWidthSurface * uHeightSurface);
            memset(pBuff + uWidthSurface * uHeightSurface, 0x80, (uHeightSurface * uWidthSurface) / 2);
            xScale = 2;
            yScale = 2;
            break;
        case NvMediaSurfaceType_Image_Y16:
        case NvMediaSurfaceType_Image_Y10:
            width *= 2;
            uWidthSurface *= 2;
            YUVPitch[0] = uWidthSurface;
            imageSize = width * height;
            memset(pBuff, 0x10, uWidthSurface * uHeightSurface);
            break;
        case NvMediaSurfaceType_Image_RGBA:
            imageSize = uWidthSurface * uHeightSurface * 4;
            memset(pBuff, 0x10, imageSize);
            YUVPitch[0] = width * 4;
            break;
        case NvMediaSurfaceType_Image_RAW:
            imageSize = uWidthSurface * uHeightSurface * bytesPerPixel;
            memset(pBuff, 0xFF, imageSize);
            YUVPitch[0] = uWidthSurface * bytesPerPixel;
            break;
        case NvMediaSurfaceType_Image_Monochrome:
        default:
            LOG_ERR("ReadImage: Invalid image surface type\n");
            goto done;
    }

    if(!(file = fopen(fileName, "rb"))) {
        LOG_ERR("ReadImage: Error opening file: %s\n", fileName);
        goto done;
    }

    if(frameNum > 0) {
        if(fseeko(file, frameNum * (off_t)imageSize, SEEK_SET)) {
            LOG_ERR("ReadImage: Error seeking file: %s\n", fileName);
            goto done;
        }
    }

    YUVPitch[1] = uWidthSurface / xScale;
    YUVPitch[2] = uWidthSurface / xScale;

    pYBuff = pBuff;
    pVBuff = pYBuff + uWidthSurface * uHeightSurface;
    pUBuff = pVBuff + (uWidthSurface * uHeightSurface) / (xScale * yScale);

    pYUVBuff[0] = pYBuff;
    pYUVBuff[1] = pUBuff;
    pYUVBuff[2] = pVBuff;

    switch(image->type) {
        case NvMediaSurfaceType_Image_YUV_444:
        case NvMediaSurfaceType_Image_YUV_422:
        case NvMediaSurfaceType_Image_YUYV_422:
        case NvMediaSurfaceType_Image_YUV_420:
            //read Y U V separately
            for(i = 0; i < height; i++) {
                if(fread(pYBuff, width, 1, file) != 1) {
                    LOG_ERR("ReadImage: Error reading file: %s\n", fileName);
                    goto done;
                }
                pYBuff += uWidthSurface;
            }

            pChroma = uvOrderFlag ? pUBuff : pVBuff;
            for(i = 0; i < height / yScale; i++) {
                if(fread(pChroma, width / xScale, 1, file) != 1) {
                    LOG_ERR("ReadImage: Error reading file: %s\n", fileName);
                    goto done;
                }
                pChroma += uWidthSurface / xScale;
            }

            pChroma = uvOrderFlag ? pVBuff : pUBuff;
            for(i = 0; i < height / yScale; i++) {
                if(fread(pChroma, width / xScale, 1, file) != 1) {
                    LOG_ERR("ReadImage: Error reading file: %s\n", fileName);
                    goto done;
                }
                pChroma += uWidthSurface / xScale;
            }
            break;
        case NvMediaSurfaceType_Image_Y16:
        case NvMediaSurfaceType_Image_Y10:
            //read Y data only
            for(i = 0; i < height; i++) {
                if(fread(pYBuff, width, 1, file) != 1) {
                    LOG_ERR("ReadImage: Error reading file: %s\n", fileName);
                    goto done;
                }
                pYBuff += uWidthSurface;
            }
            break;
        case NvMediaSurfaceType_Image_RGBA:
            if(strstr(fileName, ".png")) {
                LOG_ERR("ReadImage: Use ReadPNGImage() function from common/nvmediatest_png.c\n");
                goto done;
            } else if(fread(pBuff, imageSize, 1, file) != 1) {
                LOG_ERR("ReadImage: Error reading file: %s\n", fileName);
                goto done;
            }
            break;
        case NvMediaSurfaceType_Image_RAW:
            if(fread(pBuff, imageSize, 1, file) != 1) {
                LOG_ERR("ReadImage: Error reading file: %s\n", fileName);
                goto done;
            }
            break;
        case NvMediaSurfaceType_Image_Monochrome:
        default:
            LOG_ERR("ReadImage: Invalid image surface type\n");
            goto done;
    }

    if(IsSucceed(NvMediaImagePutBits(image, NULL, (void **)pYUVBuff, YUVPitch))) {
        status = NVMEDIA_STATUS_OK;
    } else {
        LOG_ERR("%s: Failed to put bits\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
    }


done:
    if(pBuff)
        free(pBuff);

    if(file)
        fclose(file);

    if(memSurface) {
        DestroyMemSurf(memSurface);
        memSurface = NULL;
    }

    NvMediaImageUnlock(image);

    return status;
}

static NvMediaStatus
ReadImageNew(
    char *fileName,
    NvU32 frameNum,
    NvU32 width,
    NvU32 height,
    NvMediaImage *image,
    NvMediaBool uvOrderFlag,
    NvU32 bytesPerPixel)
{
    NvU8 **pBuff = NULL;
    NvU32 *pBuffPitches = NULL;
    NvU32 imageSize = 0,surfaceSize = 0;
    NvU8 *buffer = NULL;
    NvU8 *pBuffer = NULL;
    NvU32 i, j, k, newk = 0;
    float *xScalePtr = NULL, *yScalePtr = NULL;
    unsigned int *bytePerPixelPtr = NULL;
    NvU32 numSurfaces = 1;
    unsigned int uHeightSurface, uWidthSurface;
    NvMediaImageSurfaceMap surfaceMap;
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;
    FILE *file = NULL;
    unsigned int count, index;
    unsigned int surfType, surfBPC;
    NVM_SURF_FMT_DEFINE_ATTR(srcAttr);

    if(!image || !fileName) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    status = NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageLock() failed\n", __func__);
        return status;
    }
    NvMediaImageUnlock(image);

    uHeightSurface = surfaceMap.height;
    uWidthSurface  = surfaceMap.width;

    if(width > uWidthSurface || height > uHeightSurface) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    pBuff = (NvU8**)malloc(sizeof(NvU8*)*MAXM_NUM_SURFACES);
    if(!pBuff) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pBuffPitches = (NvU32*)calloc(1,sizeof(NvU32) * MAXM_NUM_SURFACES);
    if(!pBuffPitches) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    status = GetSurfParams(image->type,
                           &xScalePtr,
                           &yScalePtr,
                           &bytePerPixelPtr,
                           &numSurfaces);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: GetSurfParams() failed\n", __func__);
        goto done;
    }

    status = NvMediaSurfaceFormatGetAttrs(image->type,
                                          srcAttr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        goto done;
    }

    surfType = srcAttr[NVM_SURF_ATTR_SURF_TYPE].value;
    surfBPC = srcAttr[NVM_SURF_ATTR_BITS_PER_COMPONENT].value;

    surfaceSize = 0;
    imageSize = 0;
    for(i = 0; i < numSurfaces; i++) {
        surfaceSize += (uWidthSurface * xScalePtr[i] * uHeightSurface * yScalePtr[i] * bytePerPixelPtr[i]);
        imageSize += (width * xScalePtr[i] * height * yScalePtr[i] * bytePerPixelPtr[i]);
        pBuffPitches[i] = (NvU32)((float)uWidthSurface * xScalePtr[i]) * bytePerPixelPtr[i];
    }

    buffer = (NvU8*)calloc(1, surfaceSize);
    if(!buffer) {
        LOG_ERR("%s: Out of memory\n", __func__);
        status = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    pBuffer = buffer;
    memset(buffer,0x10,surfaceSize);
    for(i = 0; i < numSurfaces; i++) {
        pBuff[i] = buffer;
        if (i) {
            memset(pBuff[i], 0x80, (uHeightSurface * yScalePtr[i] * pBuffPitches[i]));
        }
        buffer = buffer + (NvU32)(uHeightSurface * yScalePtr[i] * pBuffPitches[i]);
    }

    file = fopen(fileName, "rb");
    if(!file) {
        LOG_ERR("%s: Error opening file: %s\n", __func__, fileName);
        status = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    if(frameNum > 0) {
        if(fseeko(file, frameNum * (off_t)imageSize, SEEK_SET)) {
            LOG_ERR("ReadImage: Error seeking file: %s\n", fileName);
            status = NVMEDIA_STATUS_ERROR;
            goto done;
        }
    }

    if((image->type == NvMediaSurfaceType_Image_RGBA) && strstr(fileName, ".png")) {
        LOG_ERR("ReadImage: Use ReadPNGImage() function from common/nvmediatest_png.c\n");
        status = NVMEDIA_STATUS_ERROR;
        goto done;
    }
    for(k = 0; k < numSurfaces; k++) {
        for(j = 0; j < height*yScalePtr[k]; j++) {
            newk = (!uvOrderFlag && k ) ? (numSurfaces - k) : k;
            index = j * pBuffPitches[newk];
            count = width * xScalePtr[newk] * bytePerPixelPtr[newk];
            if (fread(pBuff[newk] + index, count, 1, file) != 1) {
                status = NVMEDIA_STATUS_ERROR;
                LOG_ERR("ReadImage: Error reading file: %s\n", fileName);
                goto done;
            }
            if(surfType == NVM_SURF_ATTR_SURF_TYPE_YUV) {
                NvU16 *psrc = (NvU16*)(pBuff[newk] + index);
                switch(surfBPC) {
                    case NVM_SURF_ATTR_BITS_PER_COMPONENT_10:
                        for(i = 0; i < count/2; i++) {
                            *(psrc + i) = (*(psrc + i)) << (16 - 10);
                        }
                        break;
                    case NVM_SURF_ATTR_BITS_PER_COMPONENT_12:
                        for(i = 0; i < count/2; i++) {
                            *(psrc + i) = (*(psrc + i)) << (16 - 12);
                        }
                        break;
                    case NVM_SURF_ATTR_BITS_PER_COMPONENT_14:
                        for(i = 0; i < count/2; i++) {
                            *(psrc + i) = (*(psrc + i)) << (16 - 14);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }

    status = NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageLock() failed\n", __func__);
        goto done;
    }
    status = NvMediaImagePutBits(image, NULL, (void **)pBuff, pBuffPitches);
    NvMediaImageUnlock(image);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to put bits\n", __func__);
    }

done:
    if(pBuff) {
        free(pBuff);
    }

    if (pBuffPitches) {
        free(pBuffPitches);
    }

    if (pBuffer) {
        free(pBuffer);
    }

    if(file) {
        fclose(file);
    }

    return status;
}

NvMediaStatus
ReadImage(
    char *fileName,
    NvU32 frameNum,
    NvU32 width,
    NvU32 height,
    NvMediaImage *image,
    NvMediaBool uvOrderFlag,
    NvU32 bytesPerPixel)
{
    NvMediaStatus status;
    NVM_SURF_FMT_DEFINE_ATTR(srcAttr);

    status = NvMediaSurfaceFormatGetAttrs(image->type,
                                          srcAttr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status == NVMEDIA_STATUS_NOT_SUPPORTED) {
        return ReadImageOld(
                        fileName,
                        frameNum,
                        width,
                        height,
                        image,
                        uvOrderFlag,
                        bytesPerPixel);
    } else if (status == NVMEDIA_STATUS_OK) {
        return ReadImageNew(
                        fileName,
                        frameNum,
                        width,
                        height,
                        image,
                        uvOrderFlag,
                        bytesPerPixel);
    } else {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return status;
    }

}

NvMediaStatus
InitImage(
    NvMediaImage *image,
    NvU32 bytesPerPixel)
{
    NvU32 xScale = 1, yScale = 1;
    NvU32 uHeightSurface, uWidthSurface, YUVPitch[3];
    NvU8 *pYUVBuff[3], *pBuff = NULL, *pYBuff, *pUBuff = NULL, *pVBuff = NULL;
    NvMediaImageSurfaceMap surfaceMap;
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;

    if(NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap) != NVMEDIA_STATUS_OK) {
        LOG_ERR("InitImage: NvMediaImageLock failed\n");
        goto done;
    }

    uHeightSurface = surfaceMap.height;
    uWidthSurface  = surfaceMap.width;

    pBuff = (NvU8*)calloc(1, uHeightSurface * uWidthSurface * 4);
    if(!pBuff) {
        LOG_ERR("InitImage: Failed to allocate image buffer\n");
        return NVMEDIA_STATUS_OUT_OF_MEMORY;
    }

    switch(image->type) {
        case NvMediaSurfaceType_Image_YUV_444:
            YUVPitch[0] = uWidthSurface;
            //YVU order in the buffer
            xScale = 1;
            yScale = 1;
            break;
        case NvMediaSurfaceType_Image_YUV_422:
        case NvMediaSurfaceType_Image_YUYV_422:
            YUVPitch[0] = uWidthSurface;
            xScale = 2;
            yScale = 1;
            break;
        case NvMediaSurfaceType_Image_YUV_420:
            YUVPitch[0] = uWidthSurface;
            xScale = 2;
            yScale = 2;
            break;
        case NvMediaSurfaceType_Image_RGBA:
            YUVPitch[0] = uWidthSurface * 4;
            break;
        case NvMediaSurfaceType_Image_RAW:
            YUVPitch[0] = uWidthSurface * bytesPerPixel;
            break;
        case NvMediaSurfaceType_Image_Monochrome:
        case NvMediaSurfaceType_Image_V16Y16U16X16:
        case NvMediaSurfaceType_Image_X2U10Y10V10:
        default:
            goto done;
    }

    YUVPitch[1] = uWidthSurface / xScale;
    YUVPitch[2] = uWidthSurface / xScale;

    pYBuff = pBuff;
    pVBuff = pYBuff + uWidthSurface * uHeightSurface;
    pUBuff = pVBuff + (uWidthSurface * uHeightSurface) / (xScale * yScale);

    pYUVBuff[0] = pYBuff;
    pYUVBuff[1] = pUBuff;
    pYUVBuff[2] = pVBuff;

    if(IsSucceed(NvMediaImagePutBits(image, NULL, (void **)pYUVBuff, YUVPitch))) {
        status = NVMEDIA_STATUS_OK;
    } else {
        LOG_ERR("%s: Failed to put bits\n", __func__);
        status = NVMEDIA_STATUS_ERROR;
    }

done:
    if(pBuff)
        free(pBuff);

    NvMediaImageUnlock(image);

    return status;
}

NvMediaStatus
ReadPPMImage(
    char *fileName,
    NvMediaImage *image)
{
    NvU32 uSurfaceWidth, uSurfaceHeight;
    NvU32 x, y;
    char buf[256], *c;
    NvU8 *pBuff = NULL, *pRGBBuff;
    NvU32 uFrameSize;
    FILE *file = NULL;
    NvMediaImageSurfaceMap surfaceMap;
    NvMediaStatus ret = NVMEDIA_STATUS_OK;

    if(NvMediaImageLock(image, NVMEDIA_IMAGE_ACCESS_WRITE, &surfaceMap) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: NvMediaImageLock failed\n", __func__);
        goto done;
    }

    if(!image) {
        LOG_ERR("%s: Failed allocating memory\n", __func__);
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    file = fopen(fileName, "rb");
    if(!file) {
        LOG_ERR("%s: Error opening file: %s\n", __func__, fileName);
        return NVMEDIA_STATUS_ERROR;
    }

    c = fgets(buf, 256, file);
    if(!c || strncmp(buf, "P6\n", 3)) {
        LOG_ERR("%s: Invalid PPM header in file: %s\n", __func__, fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }
    do {
        c = fgets(buf, 255, file);
        if(!c) {
            LOG_ERR("%s: Invalid PPM header in file: %s\n", __func__, fileName);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        }
    } while(!strncmp(buf, "#", 1));
    if(sscanf(buf, "%u %u", &uSurfaceWidth, &uSurfaceHeight) != 2) {
        LOG_ERR("%s: Error getting PPM file resolution in file: %s\n", __func__, fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }
    if(uSurfaceWidth < 16 || uSurfaceWidth > 2048 ||
            uSurfaceHeight < 16 || uSurfaceHeight > 2048) {
        LOG_ERR("%s: Invalid PPM file resolution: %ux%u for file: %s\n", __func__, uSurfaceWidth, uSurfaceHeight, fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    uFrameSize = uSurfaceWidth * uSurfaceHeight * 3;
    pBuff = (NvU8*)malloc(uFrameSize);
    if(!pBuff) {
        LOG_ERR("%s: Out of memory\n", __func__);
        ret = NVMEDIA_STATUS_OUT_OF_MEMORY;
        goto done;
    }
    pRGBBuff = pBuff;

    if(fread(pRGBBuff, uFrameSize, 1, file) != 1) {
        LOG_ERR("%s: Error reading file: %s\n", __func__, fileName);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    for(y = 0; y < uSurfaceHeight; y++) {
        NvU8 *pPixel = (NvU8 *)surfaceMap.surface[0].mapping + surfaceMap.surface[0].pitch * y;
        for(x = 0; x < uSurfaceWidth; x++) {
            *pPixel++ = *pRGBBuff++; // R
            *pPixel++ = *pRGBBuff++; // G
            *pPixel++ = *pRGBBuff++; // B
            *pPixel++ = 255;         // Alpha
        }
    }

done:
    NvMediaImageUnlock(image);
    if(pBuff) free(pBuff);
    if(file) fclose(file);

    return ret;
}
NvMediaStatus
ReadYUVBuffer(
    FILE *file,
    NvU32 frameNum,
    NvU32 width,
    NvU32 height,
    NvU8 *pBuff,
    NvMediaBool bOrderUV)
{
    NvU8 *pYBuff, *pUBuff, *pVBuff, *pChroma;
    NvU32 frameSize = (width * height *3)/2;
    NvMediaStatus ret = NVMEDIA_STATUS_OK;
    unsigned int i;

    if(!pBuff || !file)
        return NVMEDIA_STATUS_BAD_PARAMETER;

    pYBuff = pBuff;

    //YVU order in the buffer
    pVBuff = pYBuff + width * height;
    pUBuff = pVBuff + width * height / 4;

    if(fseek(file, frameNum * frameSize, SEEK_SET)) {
        LOG_ERR("ReadYUVFrame: Error seeking file: %p\n", file);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }
    //read Y U V separately
    for(i = 0; i < height; i++) {
        if(fread(pYBuff, width, 1, file) != 1) {
            LOG_ERR("ReadYUVFrame: Error reading file: %p\n", file);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        }
        pYBuff += width;
    }

    pChroma = bOrderUV ? pUBuff : pVBuff;
    for(i = 0; i < height / 2; i++) {
        if(fread(pChroma, width / 2, 1, file) != 1) {
            LOG_ERR("ReadYUVFrame: Error reading file: %p\n", file);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        }
        pChroma += width / 2;
    }

    pChroma = bOrderUV ? pVBuff : pUBuff;
    for(i = 0; i < height / 2; i++) {
        if(fread(pChroma, width / 2, 1, file) != 1) {
            LOG_ERR("ReadYUVFrame: Error reading file: %p\n", file);
            ret = NVMEDIA_STATUS_ERROR;
            goto done;
        }
        pChroma += width / 2;
    }

done:
    return ret;
}

NvMediaStatus
ReadRGBABuffer(
    FILE *file,
    NvU32 frameNum,
    NvU32 width,
    NvU32 height,
    NvU8 *pBuff)
{
    NvU32 frameSize = width * height * 4;
    NvMediaStatus ret = NVMEDIA_STATUS_OK;

    if(!pBuff || !file)
        return NVMEDIA_STATUS_BAD_PARAMETER;

    if(fseek(file, frameNum * frameSize, SEEK_SET)) {
        LOG_ERR("ReadYUVFrame: Error seeking file: %p\n", file);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

    //read rgba data
    if(fread(pBuff, frameSize, 1, file) != 1) {
        if (feof(file))
            LOG_DBG("ReadRGBAFrame: file read to the end\n");
        else
            LOG_ERR("ReadRGBAFrame: Error reading file: %p\n", file);
        ret = NVMEDIA_STATUS_ERROR;
        goto done;
    }

done:
    return ret;
}

NvMediaStatus
GetFrameCrc(
    NvU8   **pBuff,
    NvU32  *widths,
    NvU32  *heights,
    NvU32  *pitches,
    NvU32  numSurfaces,
    NvU32  *crcOut)
{
    NvU32 lines, crc = 0;
    NvU32 i;
    NvU8 *bufferTmp;

    if(!pBuff || !crcOut || !widths || !heights || !pitches || (numSurfaces == 0)) {
        LOG_ERR("GetFrameCrc: Bad parameter\n");
        return NVMEDIA_STATUS_BAD_PARAMETER;
    }

    for(i = 0; i < numSurfaces; i++) {
        lines = heights[i];
        bufferTmp = pBuff[i];
        while(lines--) {
            crc = CalculateBufferCRC(widths[i], crc, bufferTmp);
            bufferTmp += pitches[i];
        }
    }

    *crcOut = crc;
    return NVMEDIA_STATUS_OK;
}
