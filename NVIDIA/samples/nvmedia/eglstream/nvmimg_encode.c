/*
 * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "nvmimg_encode.h"

NvMediaStatus EncodeOneImageFrame(ImageInputParameters *pParams,
                                  NvMediaImage *pNvMediaImage,
                                  NvMediaRect *pSourceRect)
{
    ImageInputParameters *pInputParams = pParams;
    NvMediaIEP *h264Encoder = pInputParams->h264Encoder;
    FILE *outputFile = pInputParams->outputFile;
    NvMediaEncodePicParamsH264 *encodePicParams = &pInputParams->encodePicParams;
    NvU32 uNumBytes, uNumBytesAvailable = 0;
    NvU8 *pBuffer;

    //set one frame params, default = 0
    memset(encodePicParams, 0, sizeof(NvMediaEncodePicParamsH264));
    //IPP mode
    encodePicParams->pictureType = NVMEDIA_ENCODE_PIC_TYPE_AUTOSELECT;

    NvMediaStatus status = NvMediaIEPFeedFrame(h264Encoder,                 // *encoder
                                               pNvMediaImage,               // *frame
                                               pSourceRect,                 // *sourceRect
                                               encodePicParams,             // encoder parameter
                                               NVMEDIA_ENCODER_INSTANCE_0); // encoder instance

    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("\nNvMediaImageEncoderFeedFrame failed: %x\n", status);
        goto fail;
    }

    NvMediaBool bEncodeFrameDone = NVMEDIA_FALSE;
    while(!bEncodeFrameDone) {
        NvMediaBitstreamBuffer bitstreams = {0};
        uNumBytesAvailable = 0;
        uNumBytes = 0;
        status = NvMediaIEPBitsAvailable(h264Encoder,
                                         &uNumBytesAvailable,
                                         NVMEDIA_ENCODE_BLOCKING_TYPE_IF_PENDING,
                                         NVMEDIA_VIDEO_ENCODER_TIMEOUT_INFINITE);

        switch(status) {
            case NVMEDIA_STATUS_OK:
                //Add extra header space
                pBuffer = malloc(uNumBytesAvailable+256);
                if(!pBuffer) {
                    LOG_ERR("Error allocating %d bytes\n",
                            uNumBytesAvailable + 256);
                    goto fail;
                }
                bitstreams.bitstream = pBuffer;
                bitstreams.bitstreamSize = uNumBytesAvailable + 256;
                memset(pBuffer, 0xE5, uNumBytesAvailable);
                status = NvMediaIEPGetBitsEx(h264Encoder,
                                             &uNumBytes,
                                             1,
                                             &bitstreams,
                                             NULL);

                if(status != NVMEDIA_STATUS_OK &&
                    status != NVMEDIA_STATUS_NONE_PENDING) {
                    LOG_ERR("Error getting encoded bits\n");
                    goto fail;
                }

                if(uNumBytes != uNumBytesAvailable) {
                    LOG_ERR("Error-byte counts do not match %d vs. %d\n",
                            uNumBytesAvailable, uNumBytes);
                    goto fail;
                }
                if(fwrite(pBuffer,
                          uNumBytesAvailable,
                          1,
                          outputFile) != 1) {
                    LOG_ERR("Error writing %d bytes\n", uNumBytesAvailable);
                    goto fail;
                }
                if (pBuffer) {
                    free(pBuffer);
                    pBuffer = NULL;
                }

                bEncodeFrameDone = 1;
                break;
            case NVMEDIA_STATUS_PENDING:
                LOG_DBG("Status - pending\n");
                break;
            case NVMEDIA_STATUS_NONE_PENDING:
                LOG_ERR("Error - no encoded data is pending\n");
                goto fail;
            default:
                LOG_ERR("Error occured\n");
                goto fail;
        }
    }
    return NVMEDIA_STATUS_OK;
fail:
    return NVMEDIA_STATUS_ERROR;
}

NvMediaStatus ImageEncoderInit(ImageInputParameters *pInputParams,
                               int width,
                               int height,
                               NvMediaSurfaceType surfaceType,
                               NvMediaDevice *device)
{
    NvMediaStatus status = NVMEDIA_STATUS_OK;

    NvMediaIEP **ph264Encoder = &pInputParams->h264Encoder;
    NvMediaIEP *h264Encoder = NULL;

    NvMediaEncodeInitializeParamsH264 encoderInitParams;
    pInputParams->encodeConfigH264Params.h264VUIParameters =
                  calloc(1, sizeof(NvMediaEncodeConfigH264VUIParams));

    if(!pInputParams->encodeConfigH264Params.h264VUIParameters) {
        LOG_ERR("Error - Unable to allocate memory\n");
        goto fail;
    }

    NvU32 uFrameCounter = 0;
    LOG_DBG("Encode start from frame %d\n", uFrameCounter);

    LOG_DBG("Setting encoder initialization params\n");
    memset(&encoderInitParams, 0, sizeof(encoderInitParams));

    encoderInitParams.encodeHeight = height;
    encoderInitParams.encodeWidth = width;
    encoderInitParams.frameRateDen = 1;
    encoderInitParams.frameRateNum = 30;
    encoderInitParams.maxNumRefFrames = 2;
    encoderInitParams.enableExternalMEHints = NVMEDIA_FALSE;

    h264Encoder = NvMediaIEPCreate(device,                       // nvmedia device
                                   NVMEDIA_VIDEO_CODEC_H264,     // codec
                                   &encoderInitParams,           // init params
                                   surfaceType,                  // surfaceType
                                   0,                            // maxInputBuffering
                                   0,                            // maxOutputBuffering
                                   NVMEDIA_ENCODER_INSTANCE_0);  // encoder instance

    if(!h264Encoder) {
        LOG_ERR("NvMediaImageEncoderCreate failed\n");
        goto fail;
    }
    *ph264Encoder = h264Encoder;

    LOG_DBG("NvMediaImageEncoderCreate, %p\n", h264Encoder);

    //set RC param
    //Const QP
    pInputParams->encodeConfigH264Params.rcParams.rateControlMode = 1;
    pInputParams->encodeConfigH264Params.rcParams.numBFrames      = 0;
    //Const QP mode
    pInputParams->encodeConfigH264Params.rcParams.params.const_qp.constQP.qpIntra = 27;
    pInputParams->encodeConfigH264Params.rcParams.params.const_qp.constQP.qpInterP = 29;

    status = NvMediaIEPSetConfiguration(h264Encoder,
                                        &pInputParams->encodeConfigH264Params);

    if(status != NVMEDIA_STATUS_OK) {
       LOG_ERR("Main SetConfiguration failed\n");
       goto fail;
    }
    LOG_DBG("NvMediaImageEncoderSetConfiguration done\n");
    return NVMEDIA_STATUS_OK;
fail:
    return NVMEDIA_STATUS_ERROR;
}

NvMediaStatus ImageEncoderDeinit(ImageInputParameters *pInputParams)
{
    if(!pInputParams) {
        LOG_ERR("DeinitEncoder: Input argument is NULL\n");
        return NVMEDIA_STATUS_ERROR;
    }

    if(pInputParams->encodeConfigH264Params.h264VUIParameters) {
        free(pInputParams->encodeConfigH264Params.h264VUIParameters);
        pInputParams->encodeConfigH264Params.h264VUIParameters = NULL;
    }
    NvMediaIEPDestroy(pInputParams->h264Encoder);

    return NVMEDIA_STATUS_OK;
}
