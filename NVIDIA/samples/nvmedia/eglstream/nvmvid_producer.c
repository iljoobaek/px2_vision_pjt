/*
 * nvmvideo_producer.c
 *
 * Copyright (c) 2013-2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

//
// DESCRIPTION:   Simple video decoder EGL stream producer app
//
#include "nvmvid_producer.h"
#include "log_utils.h"

extern NvBool signal_stop;
test_video_parser_s g_parser;
#if defined(EXTENSION_LIST)
EXTENSION_LIST(EXTLST_EXTERN)
#endif
static int32_t cbBeginSequence(void *ptr, const NvMediaParserSeqInfo *pnvsi);
static NvMediaStatus cbDecodePicture(void *ptr, NvMediaParserPictureData *pd);
static NvMediaStatus cbDisplayPicture(void *ptr, NvMediaRefSurface *p, int64_t llPTS);
static void cbUnhandledNALU(void *ptr, const uint8_t *buf, int32_t size);
static NvMediaStatus cbAllocPictureBuffer(void *ptr, NvMediaRefSurface **p);
static void cbRelease(void *ptr, NvMediaRefSurface *p);
static void cbAddRef(void *ptr, NvMediaRefSurface *p);
static int Init(test_video_parser_s *parser);
static void Deinit(test_video_parser_s *parser);
static int Decode(test_video_parser_s *parser);
static NvBool DisplayInit(test_video_parser_s *parser, int width, int height, int videoWidth, int videoHeight);
static void DisplayDestroy(test_video_parser_s *parser);
static void DisplayFrame(test_video_parser_s *parser, frame_buffer_s *frame);
static void DisplayFlush(test_video_parser_s *parser);
static void UpdateNvMediaSurfacePictureInfoH264(NvMediaPictureInfoH264 *pictureInfo);

static int SendRenderSurface(test_video_parser_s *parser, NvMediaVideoSurface *renderSurface, NvMediaTime *timestamp);
static NvMediaVideoSurface *GetRenderSurface(test_video_parser_s *parser);

static int32_t cbBeginSequence(void *ptr, const NvMediaParserSeqInfo *pnvsi)
{
    test_video_parser_s *ctx = (test_video_parser_s*)ptr;

    char *chroma[] = {
        "Monochrome",
        "4:2:0",
        "4:2:2",
        "4:4:4"
    };

    if(!pnvsi || !ctx) {
        LOG_ERR("cbBeginSequence: Invalid NvMediaParserSeqInfo or VideoDemoTestCtx\n");
        return -1;
    }

    uint32_t decodeBuffers = pnvsi->uDecodeBuffers;

    if (pnvsi->eCodec != NVMEDIA_VIDEO_CODEC_H264) {
        LOG_ERR("BeginSequence: Invalid codec type: %d\n", pnvsi->eCodec);
        return 0;
    }

    LOG_DBG("BeginSequence: %dx%d (disp: %dx%d) codec: H264 decode buffers: %d aspect: %d:%d fps: %f chroma: %s\n",
        pnvsi->uCodedWidth, pnvsi->uCodedHeight, pnvsi->uDisplayWidth, pnvsi->uDisplayHeight,
        pnvsi->uDecodeBuffers, pnvsi->uDARWidth, pnvsi->uDARHeight,
        pnvsi->fFrameRate, pnvsi->eChromaFormat > 3 ? "Invalid" : chroma[pnvsi->eChromaFormat]);

    if (!ctx->frameTimeUSec && pnvsi->fFrameRate >= 5.0 && pnvsi->fFrameRate <= 120.0) {
        ctx->frameTimeUSec = 1000000.0 / pnvsi->fFrameRate;
    }

    if (!ctx->aspectRatio && pnvsi->uDARWidth && pnvsi->uDARHeight) {
        double aspect = (float)pnvsi->uDARWidth / (float)pnvsi->uDARHeight;
        if (aspect > 0.3 && aspect < 3.0)
            ctx->aspectRatio = aspect;
    }

    // Check resolution change
    if (pnvsi->uCodedWidth != ctx->decodeWidth || pnvsi->uCodedHeight != ctx->decodeHeight) {
        NvMediaVideoCodec codec;
        NvMediaSurfaceType surfType;
        NvMediaSurfAllocAttr surfAllocAttrs[8];
        NvU32 numSurfAllocAttrs= 0;
        NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);
        NVM_SURF_FMT_DEFINE_ATTR(attr);
        NvMediaStatus status;
        NvU32 maxReferences;
        int i;
        char *env;

        memset(surfAllocAttrs, 0, sizeof(NvMediaSurfAllocAttr)*8);

        LOG_DBG("BeginSequence: Resolution changed: Old:%dx%d New:%dx%d\n",
            ctx->decodeWidth, ctx->decodeHeight, pnvsi->uCodedWidth, pnvsi->uCodedHeight);

        ctx->decodeWidth = pnvsi->uCodedWidth;
        ctx->decodeHeight = pnvsi->uCodedHeight;

        ctx->displayWidth = pnvsi->uDisplayWidth;
        ctx->displayHeight = pnvsi->uDisplayHeight;

        ctx->renderWidth  = ctx->displayWidth;
        ctx->renderHeight = ctx->displayHeight;

        if (ctx->decoder) {
            NvMediaVideoDecoderDestroy(ctx->decoder);
        }

        LOG_DBG("Create decoder: ");
        codec = NVMEDIA_VIDEO_CODEC_H264;
        LOG_DBG("NVMEDIA_VIDEO_CODEC_H264");

        maxReferences = (decodeBuffers > 0) ? decodeBuffers - 1 : 0;
        maxReferences = (maxReferences > 16) ? 16 : maxReferences;

        LOG_DBG(" Size: %dx%d maxReferences: %d\n", ctx->decodeWidth, ctx->decodeHeight,
            maxReferences);
        ctx->decoder = NvMediaVideoDecoderCreateEx(
            ctx->device,                 // device
            codec,                       // codec
            ctx->decodeWidth,            // width
            ctx->decodeHeight,           // height
            maxReferences,               // maxReferences
            pnvsi->uMaxBitstreamSize,    //maxBitstreamSize
            5,                           // inputBuffering
            0,                           // flags
            NVMEDIA_DECODER_INSTANCE_0); // instanceId
        if (!ctx->decoder) {
            LOG_ERR("Unable to create decoder\n");
            return 0;
        }

        for(i = 0; i < MAX_FRAMES; i++) {
            if (ctx->RefFrame[i].videoSurface) {
                NvMediaVideoSurfaceDestroy(ctx->RefFrame[i].videoSurface);
            }
        }

        memset(&ctx->RefFrame[0], 0, sizeof(frame_buffer_s) * MAX_FRAMES);

        switch (pnvsi->eChromaFormat) {
            case 0: // Monochrome
            case 1: // 4:2:0
                LOG_DBG("Chroma format: NvMediaSurfaceType YUV 420\n");
                NVM_SURF_FMT_SET_ATTR_YUV(surfFormatAttrs,YUV,420,SEMI_PLANAR,UINT,8,BL);
                break;
            case 2: // 4:2:2
                LOG_DBG("Chroma format: NvMediaSurfaceType YUV 422\n");
                NVM_SURF_FMT_SET_ATTR_YUV(surfFormatAttrs,YUV,422,SEMI_PLANAR,UINT,8,BL);
                break;
            case 3: // 4:4:4
                LOG_DBG("Chroma format: NvMediaSurfaceType YUV 444\n");
                NVM_SURF_FMT_SET_ATTR_YUV(surfFormatAttrs,YUV,444,SEMI_PLANAR,UINT,8,BL);
                break;
            default:
                LOG_DBG("Invalid chroma format: %d\n", pnvsi->eChromaFormat);
                return 0;
        }

        surfType = NvMediaSurfaceFormatGetType(surfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);
        ctx->nBuffers = decodeBuffers + MAX_DISPLAY_BUFFERS;
        surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
        surfAllocAttrs[0].value = (pnvsi->uCodedWidth + 15) & ~15;
        surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
        surfAllocAttrs[1].value = (pnvsi->uCodedHeight + 15) & ~15;
        surfAllocAttrs[2].type = NVM_SURF_ATTR_CPU_ACCESS;
        surfAllocAttrs[2].value = NVM_SURF_ATTR_CPU_ACCESS_UNMAPPED;
        numSurfAllocAttrs = 3;

        if(ctx->directFlip) {
            surfAllocAttrs[3].type = NVM_SURF_ATTR_ALLOC_TYPE;
            surfAllocAttrs[3].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
            numSurfAllocAttrs += 1;
            if ((env = getenv("DISPLAY_VM"))) {
                surfAllocAttrs[4].type = NVM_SURF_ATTR_PEER_VM_ID;
                surfAllocAttrs[4].value = atoi(env);
                numSurfAllocAttrs += 1;
            }
        }

#ifndef NVMEDIA_GHSI
        else if((ctx->crossPartProducer == NV_TRUE)) {
            surfAllocAttrs[numSurfAllocAttrs].type = NVM_SURF_ATTR_ALLOC_TYPE;
            surfAllocAttrs[numSurfAllocAttrs].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
            numSurfAllocAttrs += 1;
            surfAllocAttrs[numSurfAllocAttrs].type = NVM_SURF_ATTR_PEER_VM_ID;
            surfAllocAttrs[numSurfAllocAttrs].value = ctx->consumerVmId;
            numSurfAllocAttrs += 1;
        }
#endif

        for(i = 0; i < ctx->nBuffers; i++) {
            ctx->RefFrame[i].videoSurface =
                NvMediaVideoSurfaceCreateNew(
                    ctx->device,
                    surfType,
                    surfAllocAttrs,
                    numSurfAllocAttrs,
                    0);
            if (!ctx->RefFrame[i].videoSurface) {
                LOG_ERR("Unable to create video surface\n");
                return 0;
            }
            LOG_DBG("Create video surface[%d]: %dx%d\n Ptr:%p Surface:%p Device:%p\n", i,
                (pnvsi->uCodedWidth + 15) & ~15, (pnvsi->uCodedHeight + 15) & ~15,
                &ctx->RefFrame[i], ctx->RefFrame[i].videoSurface, ctx->device);
        }

        status = NvMediaSurfaceFormatGetAttrs(ctx->surfaceType,
                                              attr,
                                              NVM_SURF_FMT_ATTR_MAX);
        if (status != NVMEDIA_STATUS_OK) {
            LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
            return NV_FALSE;
        }

        surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
        surfAllocAttrs[0].value = ctx->renderWidth;
        surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
        surfAllocAttrs[1].value = ctx->renderHeight;
        surfAllocAttrs[2].type = NVM_SURF_ATTR_CPU_ACCESS;
        surfAllocAttrs[2].value = NVM_SURF_ATTR_CPU_ACCESS_UNMAPPED;
        numSurfAllocAttrs = 3;

        if(ctx->directFlip) {
            surfAllocAttrs[3].type = NVM_SURF_ATTR_ALLOC_TYPE;
            surfAllocAttrs[3].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
            numSurfAllocAttrs += 1;
            if ((env = getenv("DISPLAY_VM"))) {
                surfAllocAttrs[4].type = NVM_SURF_ATTR_PEER_VM_ID;
                surfAllocAttrs[4].value = atoi(env);
                numSurfAllocAttrs += 1;
            }
        }

#ifndef NVMEDIA_GHSI
        else if((ctx->crossPartProducer == NV_TRUE)) {
            surfAllocAttrs[numSurfAllocAttrs].type = NVM_SURF_ATTR_ALLOC_TYPE;
            surfAllocAttrs[numSurfAllocAttrs].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
            numSurfAllocAttrs += 1;
            surfAllocAttrs[numSurfAllocAttrs].type = NVM_SURF_ATTR_PEER_VM_ID;
            surfAllocAttrs[numSurfAllocAttrs].value = ctx->consumerVmId;
            numSurfAllocAttrs += 1;
        }
#endif

        if(attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
            for (i = 0; i < MAX_RENDER_SURFACE; i++) {
                ctx->renderSurfaces[i] =
                    NvMediaVideoSurfaceCreateNew(
                        ctx->device,
                        ctx->surfaceType,
                        surfAllocAttrs,
                        numSurfAllocAttrs,
                        0);
                if(!ctx->renderSurfaces[i]) {
                    LOG_DBG("Unable to create render surface\n");
                    return NV_FALSE;
                }
                ctx->freeRenderSurfaces[i] = ctx->renderSurfaces[i];
            }
        }
        DisplayDestroy(ctx);
        DisplayInit(ctx, ctx->displayWidth, ctx->displayHeight, pnvsi->uCodedWidth, pnvsi->uCodedHeight);
        LOG_DBG("cbBeginSequence: DisplayInit done: Mixer:%p\n", ctx->mixer);
    } else {
        printf("cbBeginSequence: No resolution change\n");
    }

    return decodeBuffers;
}

static void UpdateNvMediaSurfacePictureInfoH264(NvMediaPictureInfoH264 *pictureInfo)
{
    NvU32 i;

    for (i = 0; i < 16; i++)
    {
        NvMediaReferenceFrameH264* dpb_out = &pictureInfo->referenceFrames[i];
        frame_buffer_s* picbuf = (frame_buffer_s *)dpb_out->surface;
        dpb_out->surface = picbuf ? picbuf->videoSurface : NULL;
    }
}

static NvMediaStatus cbDecodePicture(void *ptr, NvMediaParserPictureData *pd)
{
    test_video_parser_s *ctx = (test_video_parser_s*)ptr;
    NvMediaStatus status;
    frame_buffer_s *targetBuffer = NULL;


    if(!pd || !ctx) {
        LOG_ERR("cbDecodePicture: Invalid NvMediaParserPictureData or test_video_parser_s\n");
        return NVMEDIA_STATUS_ERROR;
    }

    LOG_DBG("cbDecodePicture: Mixer:%p\n", ctx->mixer);

    if (pd->pCurrPic) {
        NvMediaBitstreamBuffer bitStreamBuffer[1];

        NvU64 timeEnd, timeStart;
        GetTimeMicroSec(&timeStart);

        targetBuffer = (frame_buffer_s *)pd->pCurrPic;
        UpdateNvMediaSurfacePictureInfoH264((NvMediaPictureInfoH264*)&pd->CodecSpecificInfo.h264);

        targetBuffer->frameNum = ctx->nPicNum;

        bitStreamBuffer[0].bitstream = (NvU8 *)pd->pBitstreamData;
        bitStreamBuffer[0].bitstreamBytes = pd->uBitstreamDataLen;

        LOG_DBG("DecodePicture %d Ptr:%p Surface:%p (stream ptr:%p size: %d)...\n",
            ctx->nPicNum, targetBuffer, targetBuffer->videoSurface, pd->pBitstreamData, pd->uBitstreamDataLen);
        ctx->nPicNum++;

        if (targetBuffer->videoSurface) {
            status = NvMediaVideoDecoderRenderEx(
                ctx->decoder,                                 // decoder
                targetBuffer->videoSurface,                   // target
                (NvMediaPictureInfo *)&pd->CodecSpecificInfo, // pictureInfo
                NULL,                                         // encryptParams
                1,                                            // numBitstreamBuffers
                &bitStreamBuffer[0],                          // bitstreams
                NULL,                                         // FrameStatsDump
                NVMEDIA_DECODER_INSTANCE_0);                  // Instance ID
            if (status != NVMEDIA_STATUS_OK) {
                LOG_ERR("Decode Picture: Decode failed: %d\n", status);
                return NVMEDIA_STATUS_ERROR;
            }
            LOG_DBG("Decode Picture: Decode done\n");
        } else {
            LOG_ERR("Decode Picture: Invalid target surface\n");
        }

        GetTimeMicroSec(&timeEnd);

        ctx->decodeCount++;

    } else {
        LOG_ERR("Decode Picture: No valid frame\n");
        return NVMEDIA_STATUS_ERROR;
    }

    return NVMEDIA_STATUS_OK;
}

static NvMediaStatus cbDisplayPicture(void *ptr, NvMediaRefSurface *p, int64_t llPTS)
{
    test_video_parser_s *ctx = (test_video_parser_s*)ptr;
    frame_buffer_s* buffer = (frame_buffer_s*)p;

    if(!ctx) {
        LOG_ERR("Display: Invalid test_video_parser_s \n");
        return NVMEDIA_STATUS_ERROR;
    }

    if (p) {
        LOG_DBG("cbDisplayPicture: %d ctx:%p frame_buffer:%p Surface:%p\n", buffer->frameNum, ctx, buffer, buffer->videoSurface);
        DisplayFrame(ctx, buffer);
    } else if(!ctx->stopDecoding) {
        LOG_ERR("Display: Invalid buffer\n");
        return NVMEDIA_STATUS_ERROR;
    }
    return NVMEDIA_STATUS_OK;
}

static void cbUnhandledNALU(void *ptr, const uint8_t *buf, int32_t size)
{

}

static void ReleaseFrame(test_video_parser_s *parser, NvMediaVideoSurface *videoSurface)
{
    int i;
    frame_buffer_s *p;

    for (i = 0; i < MAX_FRAMES; i++) {
        p = &parser->RefFrame[i];
        if (videoSurface == p->videoSurface) {
            cbRelease((void *)parser, (NvMediaRefSurface *)p);
            break;
        }
    }
}

static NvMediaStatus cbAllocPictureBuffer(void *ptr, NvMediaRefSurface **p)
{
    int i;
    test_video_parser_s *ctx = (test_video_parser_s*)ptr;
    NvMediaStatus status;
    NVM_SURF_FMT_DEFINE_ATTR(attr);

    if(!ctx) {
        LOG_ERR("cbAllocPictureBuffer: Invalid test_video_parser_s\n");
        return NVMEDIA_STATUS_ERROR;
    }
    LOG_DBG("cbAllocPictureBuffer: Mixer:%p\n", ctx->mixer);
    *p = (NvMediaRefSurface *) NULL;
    status = NvMediaSurfaceFormatGetAttrs(ctx->surfaceType,
                                          attr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }

    while(!ctx->stopDecoding && !signal_stop) {
        for (i = 0; i < ctx->nBuffers; i++) {
            if (!ctx->RefFrame[i].nRefs) {
                *p = (NvMediaRefSurface *) &ctx->RefFrame[i];
                ctx->RefFrame[i].nRefs++;
                ctx->RefFrame[i].index = i;
                LOG_DBG("Alloc picture index: %d Ptr:%p Surface:%p\n", i, *p, ctx->RefFrame[i].videoSurface);
                return NVMEDIA_STATUS_OK;
            }
        }
        if(attr[NVM_SURF_ATTR_SURF_TYPE].value != NVM_SURF_ATTR_SURF_TYPE_RGBA) {
            //! [docs_eglstream:producer_gets_surface]
            NvMediaVideoSurface *videoSurface = NULL;
            NvMediaStatus status;
            status = NvMediaEglStreamProducerGetSurface(ctx->producer, &videoSurface, 100);
            if(status == NVMEDIA_STATUS_TIMED_OUT) {
                LOG_DBG("cbAllocPictureBuffer: NvMediaGetSurface waiting\n");
            } else if(status == NVMEDIA_STATUS_OK) {
                ReleaseFrame(ctx, videoSurface);
            } else
                goto failed;
            //! [docs_eglstream:producer_gets_surface]
        }
    }
failed:
    if(!ctx->stopDecoding && !signal_stop)
        LOG_ERR("Alloc picture failed\n");
    return NVMEDIA_STATUS_ERROR;
}

static void cbRelease(void *ptr, NvMediaRefSurface *p)
{
    frame_buffer_s* buffer = (frame_buffer_s*)p;

    if(!buffer) {
        LOG_ERR("cbRelease: Invalid FrameBuffer\n");
    }

    LOG_DBG("Release picture: %d index: %d\n", buffer->frameNum, buffer->index);

    if (buffer->nRefs > 0)
        buffer->nRefs--;
}

static void cbAddRef(void *ptr, NvMediaRefSurface *p)
{
    frame_buffer_s* buffer = (frame_buffer_s*)p;

    if(!buffer) {
        LOG_ERR("cbAddRef: Invalid FrameBuffer\n");
    }

    LOG_DBG("AddRef picture: %d\n", buffer->frameNum);

    buffer->nRefs++;
}

NvMediaParserClientCb TestClientCb =
{
    &cbBeginSequence,
    &cbDecodePicture,
    &cbDisplayPicture,
    &cbUnhandledNALU,
    &cbAllocPictureBuffer,
    &cbRelease,
    &cbAddRef
};

static int Init(test_video_parser_s *parser)
{
    float defaultFrameRate = 30.0;

    printf("Init: Opening file: %s\n", parser->filename);

    parser->fp = fopen(parser->filename, "rb");
    if (!parser->fp) {
        printf("failed to open stream %s\n", parser->filename);
        return 0;
    }

    memset(&parser->nvsi, 0, sizeof(parser->nvsi));
    parser->lDispCounter = 0;
    parser->eCodec = NVMEDIA_VIDEO_CODEC_H264;

    // create video parser
    memset(&parser->nvdp, 0, sizeof(NvMediaParserParams));
    parser->nvdp.pClient = &TestClientCb;
    parser->nvdp.pClientCtx = parser;
    parser->nvdp.uErrorThreshold = 50;
    parser->nvdp.uReferenceClockRate = 0;
    parser->nvdp.eCodec = parser->eCodec;

    LOG_DBG("Init: video_parser_create\n");
    parser->ctx = NvMediaParserCreate(&parser->nvdp);
    if (!parser->ctx) {
        LOG_ERR("NvMediaParserCreate failed\n");
        return 0;
    }

    NvMediaParserSetAttribute(parser->ctx,
                    NvMParseAttr_SetDefaultFramerate,
                    sizeof(float), &defaultFrameRate);

    LOG_DBG("Init: NvMediaDeviceCreate\n");
    parser->device = NvMediaDeviceCreate();
    if (!parser->device) {
        LOG_DBG("Unable to create device\n");
        return NV_FALSE;
    }

    parser->producer = NvMediaEglStreamProducerCreate(
        parser->device,
        parser->eglDisplay,
        parser->eglStream,
        parser->surfaceType,
        parser->renderWidth,
        parser->renderHeight);
    if(!parser->producer) {
        LOG_DBG("Unable to create producer\n");
        return NV_FALSE;
    }

    return 1;
}

static void Deinit(test_video_parser_s *parser)
{
    NvU32 i;

    NvMediaParserDestroy(parser->ctx);

    if (parser->fp) {
        fclose(parser->fp);
        parser->fp = NULL;
    }

    for(i = 0; i < MAX_FRAMES; i++) {
        if (parser->RefFrame[i].videoSurface) {
            NvMediaVideoSurfaceDestroy(parser->RefFrame[i].videoSurface);
            parser->RefFrame[i].videoSurface = NULL;
        }
    }

    if (parser->decoder) {
        NvMediaVideoDecoderDestroy(parser->decoder);
        parser->decoder = NULL;
    }

    DisplayDestroy(parser);

    for (i = 0; i < MAX_RENDER_SURFACE; i++) {
        if(parser->renderSurfaces[i]) {
            NvMediaVideoSurfaceDestroy(parser->renderSurfaces[i]);
            parser->renderSurfaces[i] = NULL;
        }
    }

    if(parser->producer) {
        NvMediaEglStreamProducerDestroy(parser->producer);
        parser->producer = NULL;
    }

    if (parser->device) {
        NvMediaDeviceDestroy(parser->device);
        parser->device = NULL;
    }
}

static int Decode(test_video_parser_s *parser)
{
    NvU8 *bits;
    int i;
    NvU32 readSize = READ_SIZE;

    printf("Decode start\n");

    rewind(parser->fp);

    bits = malloc(readSize);
    if (!bits) {
        return 0;
    }

    for(i = 0; (i < parser->loop) || (parser->loop == -1); i++) {
        while (!feof(parser->fp) && !parser->stopDecoding && !signal_stop) {
            size_t len;
            NvMediaBitStreamPkt packet;

            memset(&packet, 0, sizeof(NvMediaBitStreamPkt));

            len = fread(bits, 1, readSize, parser->fp);

            packet.uDataLength = (uint32_t) len;
            packet.pByteStream = bits;
            packet.bEOS = feof(parser->fp);
            packet.bPTSValid = 0;
            packet.llPts = 0;
            if (NvMediaParserParse(parser->ctx, &packet) != NVMEDIA_STATUS_OK) {
                LOG_ERR("Decode: NvMediaParserParse returned with failure\n");
                return -1;
            }
        }
        NvMediaParserFlush(parser->ctx);
        DisplayFlush(parser);
        rewind(parser->fp);

        if(parser->loop != 1 && !signal_stop) {
            if(parser->stopDecoding) {
                parser->stopDecoding = NV_FALSE;
                parser->decodeCount = 0;
            }
            printf("loop count: %d/%d \n", i+1, parser->loop);
        } else
            break;
    }

    if (bits) {
        free(bits);
        bits = NULL;
    }

    return 1;
}

static NvBool DisplayInit(test_video_parser_s *parser, int width, int height, int videoWidth, int videoHeight)
{
    unsigned int features =  0;
    float aspectRatio = (float)width / (float)height;
    NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);
    NvMediaSurfaceType surfType;

    if (parser->aspectRatio != 0.0) {
        aspectRatio = parser->aspectRatio;
    }

    LOG_DBG("DisplayInit: %dx%d Aspect: %f\n", width, height, aspectRatio);

    /* default Deinterlace: Off/Weave */

    // Set default 4:2:0 video surface type
    NVM_SURF_FMT_SET_ATTR_YUV(surfFormatAttrs, YUV, 420, SEMI_PLANAR, UINT, 8, BL);
    surfType = NvMediaSurfaceFormatGetType(surfFormatAttrs, 7);

    LOG_DBG("DisplayInit: Surface Renderer Mixer create\n");
    parser->mixer = NvMediaVideoMixerCreate(
        parser->device,       // device,
        surfType,             // surfaceType
        parser->renderWidth,  // mixerWidth
        parser->renderHeight, // mixerHeight
        aspectRatio,          // sourceAspectRatio
        videoWidth,           // primaryVideoWidth
        videoHeight,          // primaryVideoHeight
        features);            // features
    if (!parser->mixer) {
        LOG_ERR("Unable to create mixer\n");
        return NV_FALSE;
    }

    LOG_DBG("DisplayInit: Mixer:%p\n", parser->mixer);

    return NV_TRUE;
}

static void DisplayDestroy(test_video_parser_s *parser)
{
    if (parser->mixer) {
        NvMediaVideoMixerDestroy(parser->mixer);
        parser->mixer = NULL;
    }
}

static void DisplayFlush(test_video_parser_s *parser)
{
    NvMediaVideoSurface *renderSurface = NULL;
    int i;
    NvMediaStatus status;
    NVM_SURF_FMT_DEFINE_ATTR(attr);

    status = NvMediaSurfaceFormatGetAttrs(parser->surfaceType,
                                          attr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return;
    }

    if(parser->producer) {
        while(NvMediaEglStreamProducerGetSurface(parser->producer, &renderSurface, 0) == NVMEDIA_STATUS_OK) {
            if(attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
                for(i = 0; i < MAX_RENDER_SURFACE; i++) {
                    if(!parser->freeRenderSurfaces[i]) {
                        parser->freeRenderSurfaces[i] = renderSurface;
                        break;
                    }
                }
            } else {
                ReleaseFrame(parser, renderSurface);
            }
        }
    }
}

static int SendRenderSurface(test_video_parser_s *parser, NvMediaVideoSurface *renderSurface, NvMediaTime *timestamp)
{
    NvMediaStatus status;

    LOG_DBG("SendRenderSurface: Start\n");

    if(parser->stopDecoding)
        return 1;

    //! [docs_eglstream:producer_posts_frame]

    status = NvMediaEglStreamProducerPostSurface(parser->producer, renderSurface, timestamp);

    if(status != NVMEDIA_STATUS_OK) {
        EGLint streamState = 0;
        if(!eglQueryStreamKHR(
                parser->eglDisplay,
                parser->eglStream,
                EGL_STREAM_STATE_KHR,
                &streamState)) {
            LOG_ERR("main: eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
        }
        if(streamState != EGL_STREAM_STATE_DISCONNECTED_KHR && !parser->stopDecoding)
            LOG_ERR("SendRenderSurface: NvMediaPostSurface failed\n");
        return 0;

    //! [docs_eglstream:producer_posts_frame]
    }

    LOG_DBG("SendRenderSurface: End\n");
    return 1;
}

static NvMediaVideoSurface *GetRenderSurface(test_video_parser_s *parser)
{
    NvMediaVideoSurface *renderSurface = NULL;
    NvMediaStatus status;
    int i;
    NVM_SURF_FMT_DEFINE_ATTR(attr);

    status = NvMediaSurfaceFormatGetAttrs(parser->surfaceType,
                                          attr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return NULL;
    }

    if(attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
        for(i = 0; i < MAX_RENDER_SURFACE; i++) {
            if(parser->freeRenderSurfaces[i]) {
                renderSurface = parser->freeRenderSurfaces[i];
                parser->freeRenderSurfaces[i] = NULL;
                return renderSurface;
            }
        }
    }

    status = NvMediaEglStreamProducerGetSurface(parser->producer, &renderSurface, 100);
    if(status == NVMEDIA_STATUS_ERROR && attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
        LOG_DBG("GetRenderSurface: NvMediaGetSurface waiting\n");
    }

    return renderSurface;
}

static void DisplayFrame(test_video_parser_s *parser, frame_buffer_s *frame)
{
    NvMediaVideoDesc primaryVideo;
    NvMediaTime timeStamp;
    NvMediaStatus status;
    NvMediaRect primarySourceRect = { 0, 0, parser->displayWidth, parser->displayHeight };
    NvBool releaseflag = 1;
    NVM_SURF_FMT_DEFINE_ATTR(attr);

    if (!frame || !frame->videoSurface) {
        if(!parser->stopDecoding)
            LOG_ERR("DisplayFrame: Invalid surface\n");
        return;
    }

    status = NvMediaSurfaceFormatGetAttrs(parser->surfaceType,
                                          attr,
                                          NVM_SURF_FMT_ATTR_MAX);
    if (status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s:NvMediaSurfaceFormatGetAttrs failed\n", __func__);
        return;
    }

    LOG_DBG("DisplayFrame: parser:%p surface:%p Mixer:%p\n", parser, frame->videoSurface, parser->mixer);

    NvMediaVideoSurface *renderSurface = GetRenderSurface(parser);

    LOG_DBG("DisplayFrame: Render surface: %p\n", renderSurface);

    if(!renderSurface) {
        if(attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
             LOG_DBG("DisplayFrame: GetRenderSurface empty\n");
            return;
        } else {
            releaseflag = 0;
        }
    }

    /* Deinterlace Off/Weave */

    primaryVideo.pictureStructure = NVMEDIA_PICTURE_STRUCTURE_FRAME;
    primaryVideo.next = NULL;
    primaryVideo.current = frame->videoSurface;
    primaryVideo.previous = NULL;
    primaryVideo.previous2 = NULL;
    primaryVideo.srcRect = &primarySourceRect;
    primaryVideo.dstRect = NULL;

    LOG_DBG("DisplayFrame: parser->lDispCounter: %d\n", parser->lDispCounter);
    if (!parser->lDispCounter) {
        // Start timing at the first frame
        GetTimeUtil(&parser->baseTime);
    }

    NvAddTime(&parser->baseTime, (NvU64)((double)(parser->lDispCounter + 5) * parser->frameTimeUSec), &timeStamp);

    cbAddRef((void *)parser, (NvMediaRefSurface *)frame);

    if(attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA) {
        // Render to surface
        LOG_DBG("DisplayFrame: Render to surface\n");
        status = NvMediaVideoMixerRenderSurface(
            parser->mixer, // mixer
            renderSurface, // renderSurface
            NULL,          // background
            &primaryVideo);// primaryVideo
        if(status != NVMEDIA_STATUS_OK) {
            LOG_ERR("DisplayFrame: NvMediaVideoMixerRender failed\n");
        }
        LOG_DBG("DisplayFrame: Render to surface - Done\n");
    }

    if(!SendRenderSurface(parser,
                          attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA?
                          renderSurface: frame->videoSurface,
                          &timeStamp)) {
        parser->stopDecoding = 1;
    }
    LOG_DBG("DisplayFrame: SendRenderSurface - Done\n");

    if(releaseflag) {
        ReleaseFrame(parser, attr[NVM_SURF_ATTR_SURF_TYPE].value == NVM_SURF_ATTR_SURF_TYPE_RGBA?
                        primaryVideo.current:renderSurface);
    }

    parser->lDispCounter++;

    LOG_DBG("DisplayFrame: End\n");
}

static NvU32 DecodeThread(void *parserArg)
{
    test_video_parser_s *parser = (test_video_parser_s *)parserArg;

    LOG_DBG("DecodeThread: Init\n");
    if (!Init(parser)) {
        printf("DecodeThread - Init failed\n");
        // Signal end of decode
        *parser->decodeFinished = NV_TRUE;
        return 0;
    }

    LOG_DBG("DecodeThread: Decode\n");

    if (!Decode(parser)) {
        printf("DecodeThread - Decode failed\n");
    }

    // Signal end of decode
    *parser->decodeFinished = NV_TRUE;

    return 0;
}

int VideoDecoderInit(volatile NvBool *decodeFinished, EGLDisplay eglDisplay, EGLStreamKHR eglStream, TestArgs *args)
{
    test_video_parser_s *parser = &g_parser;

    // Set parser default parameters
    parser->filename = args->infile;
    parser->loop = args->prodLoop? args->prodLoop : 1;
    parser->eglDisplay = eglDisplay;
    parser->eglStream = eglStream;
    parser->decodeFinished = decodeFinished;
    parser->directFlip = (args->egloutputConsumer
#ifdef EGLOUTPUT_SUPPORT
                        || (args->crossConsumerType == EGLOUTPUT_CONSUMER)
#endif
                        ) ? 1 : 0;
    parser->crossPartProducer = (args->standalone == STANDALONE_CP_PRODUCER) ? 1 : 0;

    if(parser->crossPartProducer == NV_TRUE) {
        parser->consumerVmId = args->consumerVmId;
    }

    parser->surfaceType = args->prodSurfaceType;

    // Create video decoder thread
    if(IsFailed(NvThreadCreate(&parser->thread, &DecodeThread, (void *)parser, NV_THREAD_PRIORITY_NORMAL))) {
        LOG_ERR("VideoDecoderInit: Unable to create video decoder thread\n");
        return 0;
    }

    return 1;
}

void VideoDecoderDeinit() {
    test_video_parser_s *parser = &g_parser;

    LOG_DBG("main: Deinit\n");

    Deinit(parser);
}

void VideoDecoderStop() {
    test_video_parser_s *parser = &g_parser;

    if(parser->thread) {
        parser->stopDecoding = NV_TRUE;
        LOG_DBG("wait for video decoder thread exit\n");
        NvThreadDestroy(parser->thread);
    }
}

void VideoDecoderFlush() {
    test_video_parser_s *parser = &g_parser;

    DisplayFlush(parser);
}
