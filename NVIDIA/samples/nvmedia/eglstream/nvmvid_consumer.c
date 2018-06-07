/*
 * nvmvideo_consumer.c
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
// DESCRIPTION:   Simple video consumer rendering sample app
//

#include "nvm_consumer.h"
#include "log_utils.h"

#if defined(EXTENSION_LIST)
EXTENSION_LIST(EXTLST_EXTERN)
#endif

#define MAX_DISPLAY_BUFFERS  5

static void ReleaseRenderFrame( NvMediaVideoSurface **freeRenderSurfaces, NvMediaVideoSurface *renderSurface)
{
    int i;

    for (i = 0; i < MAX_DISPLAY_BUFFERS; i++) {
        if (!freeRenderSurfaces[i]) {
            freeRenderSurfaces[i] = renderSurface;
            break;
        }
    }
}

static NvMediaVideoSurface *GetRenderSurface(NvMediaVideoSurface **freeRenderSurfaces)
{
    NvMediaVideoSurface *renderSurface = NULL;
    int i;

    for (i = 0; i < MAX_DISPLAY_BUFFERS; i++) {
        if (freeRenderSurfaces[i]) {
            renderSurface = freeRenderSurfaces[i];
            freeRenderSurfaces[i] = NULL;
            break;
        }
    }

    return renderSurface;
}

static NvU32
procThreadFunc (
    void *data)
{
    test_nvmedia_consumer_display_s *display = (test_nvmedia_consumer_display_s *)data;
    NvMediaTime timeStamp;
    NvMediaVideoSurface *surface = NULL;
    NvMediaStatus status;
    NvMediaVideoMixer       *mixer = NULL;
    NvMediaVideoSurface     *renderSurfaces[MAX_DISPLAY_BUFFERS] = {NULL};
    NvMediaVideoSurface     *freeRenderSurfaces[MAX_DISPLAY_BUFFERS] = {NULL};
    NvMediaVideoSurface     *renderSurface = NULL;
    NvMediaVideoSurface *releaseOutputFrames[MAX_DISPLAY_BUFFERS] = {NULL};
    NvMediaVideoSurface **outputReleaseList = &releaseOutputFrames[0];
    NvMediaVideoDesc srcVideo;
    NvMediaVideoMixerAttributes mixerAttrib;
    int i;

    if(!display) {
        LOG_ERR("%s: Bad parameter\n", __func__);
        return 0;
    }

    LOG_DBG("NVMedia video consumer thread is active\n");
    while(!display->quit) {
        EGLint streamState = 0;
        if(!eglQueryStreamKHR(
                display->eglDisplay,
                display->eglStream,
                EGL_STREAM_STATE_KHR,
                &streamState)) {
            LOG_ERR("Nvmedia video consumer, eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
        }

        if(streamState == EGL_STREAM_STATE_DISCONNECTED_KHR) {
            LOG_DBG("Nvmedia video Consumer: - EGL_STREAM_STATE_DISCONNECTED_KHR received\n");
            display->quit = NV_TRUE;
            goto done;
        }

        //! [docs_eglstream:consumer_acquires_frame]
        if(NVMEDIA_STATUS_ERROR ==
            NvMediaEglStreamConsumerAcquireSurface(display->consumer, &surface, 16, &timeStamp)) {
            LOG_DBG("Nvmedia video Consumer: - surface acquire failed\n");
            display->quit = NV_TRUE;
            goto done;
        }
        //! [docs_eglstream:consumer_acquires_frame]

        if (surface)
        {
            NvMediaRect displayVideoSourceRect = { 0, 0, surface->width, surface->height };

            /* If it is the first call Initialize the renderSurface pool */
            if(mixer == NULL) {
                NvU32 numSurfAllocAttrs= 0;
                NvMediaSurfAllocAttr surfAllocAttrs[8];
                NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);
                float aspectRatio = (float)surface->width / (float)surface->height;
                char *env;

                mixer = NvMediaVideoMixerCreate(display->device,// device
                                         display->surfaceType,
                                         surface->width,        // mixerWidth
                                         surface->height,       // mixerHeight
                                         aspectRatio,           // sourceAspectRatio
                                         surface->width,        // videoWidth,// primaryVideoWidth
                                         surface->height,       // videoHeight,// primaryVideoHeight
                                         0);

                if (!mixer) {
                    LOG_ERR("Nvmedia video consumer: Unable to create mixer\n");
                    goto done;
                } else {
                    LOG_INFO("NvMediaVideoMixerCreate successfull\n");
                }

                if (display->yInvert == NV_TRUE) {
                    mixerAttrib.dstTransform = NVMEDIA_TRANSFORM_FLIP_VERTICAL;
                    NvMediaVideoMixerSetAttributes(mixer, NVMEDIA_VMP_ATTR_TRANSFORM, &mixerAttrib);
                }

                if (display->encodeEnable) {
                    LOG_DBG("Setting output surface type to YUV for encode\n");
                    NVM_SURF_FMT_SET_ATTR_YUV(surfFormatAttrs,YUV,420,SEMI_PLANAR,UINT,8,BL);
                } else {
                    NVM_SURF_FMT_SET_ATTR_RGBA(surfFormatAttrs,RGBA,UINT,8,PL);
                }

                surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
                surfAllocAttrs[0].value = surface->width;
                surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
                surfAllocAttrs[1].value = surface->height;
                numSurfAllocAttrs = 2;

                if(display->encodeEnable == NV_TRUE) {
                    surfAllocAttrs[numSurfAllocAttrs].type = NVM_SURF_ATTR_ALLOC_TYPE;
                    numSurfAllocAttrs += 1;
                    surfAllocAttrs[numSurfAllocAttrs].type = NVM_SURF_ATTR_CPU_ACCESS;
                    surfAllocAttrs[numSurfAllocAttrs].value = NVM_SURF_ATTR_CPU_ACCESS_UNMAPPED;
                    numSurfAllocAttrs += 1;
                } else {
                    surfAllocAttrs[numSurfAllocAttrs].type = NVM_SURF_ATTR_ALLOC_TYPE;
                    surfAllocAttrs[numSurfAllocAttrs].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
                    numSurfAllocAttrs += 1;
                    if ((env = getenv("DISPLAY_VM"))) {
                        surfAllocAttrs[numSurfAllocAttrs].type = NVM_SURF_ATTR_PEER_VM_ID;
                        surfAllocAttrs[numSurfAllocAttrs].value = atoi(env);
                        numSurfAllocAttrs += 1;
                    }
                }

                for (i = 0; i < MAX_DISPLAY_BUFFERS; i++) {
                    renderSurfaces[i] = NvMediaVideoSurfaceCreateNew(display->device,
                                            NvMediaSurfaceFormatGetType(surfFormatAttrs,
                                                                        NVM_SURF_FMT_ATTR_MAX),
                                            surfAllocAttrs,
                                            numSurfAllocAttrs,
                                            0);
                    if (!renderSurfaces[i]) {
                        LOG_ERR("Unable to create render surface\n");
                        goto done;
                    }
                    freeRenderSurfaces[i] = renderSurfaces[i];
                }
            }

            renderSurface = GetRenderSurface(freeRenderSurfaces);
            if (!renderSurface) {
                LOG_ERR("Videoconsumer: renderSurface empty\n");
                goto done;
            }
            memset(&srcVideo, 0, sizeof(NvMediaVideoDesc));
            srcVideo.pictureStructure = NVMEDIA_PICTURE_STRUCTURE_FRAME;
            srcVideo.current = surface;
            status = NvMediaVideoMixerRenderSurface(mixer,          // mixer
                                                    renderSurface,  // outputSurface
                                                    NULL,           // background
                                                    &srcVideo);     // primaryVideo
            if (status != NVMEDIA_STATUS_OK) {
                LOG_ERR("Videoconsumer: NvMediaVideoMixerRenderSurface failed\n");
                goto done;
            }

            if (display->output) {
                status = NvMediaVideoOutputFlip(display->output,
                                                renderSurface,              // videoSurface
                                                &displayVideoSourceRect,    // srcRect
                                                NULL,                       // dstRect
                                                outputReleaseList,          // releaselist
                                                &timeStamp);                // timeStamp
                if(status != NVMEDIA_STATUS_OK) {
                    LOG_ERR("main: NvMediaVideoOutputFlip failed\n");
                }
                while (*outputReleaseList) {
                    ReleaseRenderFrame(freeRenderSurfaces, *outputReleaseList++);
                }
                outputReleaseList = &releaseOutputFrames[0];
            } else if (display->encodeEnable) {
                if (!display->inputParams.h264Encoder) {
                    LOG_DBG("Nvmedia video consumer - InitEncoder\n");
                    if(InitEncoder(&display->inputParams, renderSurface->width,
                                   renderSurface->height, renderSurface->type)) {
                        LOG_ERR("Nvmedia video consumer: InitEncoder failed \n");
                        goto done;;
                    }
                }

                if(EncodeOneFrame(&display->inputParams, renderSurface, NULL)){
                    LOG_ERR("main: Encode frame %d fails \n", display->inputParams.uFrameCounter);
                    goto done;
                }
                display->inputParams.uFrameCounter++;
                ReleaseRenderFrame(freeRenderSurfaces, renderSurface);
            }

            NvMediaEglStreamConsumerReleaseSurface(display->consumer, surface);
        }
    }

done:
    if (display->output) {
        NvMediaVideoOutputFlip(display->output,    // output
                               NULL,               // videoSurface
                               NULL,               // srcRect
                               NULL,               // dstRect
                               outputReleaseList,  // releaselist
                               NULL);              // timeStamp
        while (*outputReleaseList) {
                ReleaseRenderFrame(freeRenderSurfaces, *outputReleaseList++);
        }
    }

    if(mixer)
        NvMediaVideoMixerDestroy(mixer);

    for(i = 0; i < MAX_DISPLAY_BUFFERS; i++) {
        if (renderSurfaces[i]) {
            NvMediaVideoSurfaceDestroy(renderSurfaces[i]);
        }
    }

    display->procThreadExited = NV_TRUE;
    *display->consumerDone = NV_TRUE;
    return 0;
}

int video_display_init(volatile NvBool *consumerDone,
                       test_nvmedia_consumer_display_s *display,
                       EGLDisplay eglDisplay, EGLStreamKHR eglStream,
                       TestArgs *args)
{
    NvMediaBool deviceEnabled = NVMEDIA_FALSE;
    NvMediaStatus rt;

    display->consumerDone = consumerDone;
    display->eglDisplay = eglDisplay;
    display->eglStream = eglStream;
    LOG_DBG("video_display_init: NvMediaDeviceCreate\n");
    display->device = NvMediaDeviceCreate();
    if (!display->device) {
        LOG_DBG("video_display_init: Unable to create device\n");
        return NV_FALSE;
    }

    display->surfaceType = args->consSurfaceType;
    display->yInvert = (args->producerType == GL_PRODUCER) ? NV_TRUE : NV_FALSE;

    if (NV_TRUE == args->displayEnabled) {
        LOG_DBG("video_display_init: Output create 0\n");

        // Check that the device is enabled (initialized)
        rt = CheckDisplayDeviceID(args->displayId, &deviceEnabled);
        if (rt != NVMEDIA_STATUS_OK) {
            LOG_ERR("Err: Chosen display (%d) not available\n", args->displayId);
            return 0;
        }

        display->output = NvMediaVideoOutputCreate(args->displayId, // displayId
                                                   args->windowId,  // windowId
                                                   NULL,            // outputPreference
                                                   deviceEnabled);  // alreadyCreated
        if(!display->output) {
            LOG_ERR("Unable to create output\n");
            return NV_FALSE;
        }
        LOG_DBG("video_display_init: Output create done: %p\n", display->output);
    } else {
        display->output = NULL;
    }

    display->consumer = NvMediaEglStreamConsumerCreate(
        display->device,
        display->eglDisplay,
        display->eglStream,
        args->consSurfaceType);
    if(!display->consumer) {
        LOG_DBG("video_display_init: Unable to create consumer\n");
        return NV_FALSE;
    }

    display->encodeEnable = args->nvmediaEncoder;

    if (display->encodeEnable) {
       memset(&display->inputParams, 0, sizeof(InputParameters));
       display->inputParams.outputFile = fopen(args->outfile, "w+");
       if(!(display->inputParams.outputFile)) {
            LOG_ERR("Error opening '%s' for encoder writing\n", args->outfile);
            return NV_FALSE;
       }
    }
    if(IsFailed(NvThreadCreate(&display->procThread, &procThreadFunc, (void *)display, NV_THREAD_PRIORITY_NORMAL))) {
        LOG_ERR("Nvmedia video consumer init: Unable to create process thread\n");
        display->procThreadExited = NV_TRUE;
        return NV_FALSE;
    }

    return NV_TRUE;
}

void video_display_Deinit(test_nvmedia_consumer_display_s *display)
{
    display->quit = NV_TRUE;

    if(display->inputParams.outputFile)
        fclose(display->inputParams.outputFile);
    if(display->inputParams.h264Encoder)
        NvMediaVideoEncoderDestroy(display->inputParams.h264Encoder);
    if(display->inputParams.encodeConfigH264Params.h264VUIParameters) {
        free(display->inputParams.encodeConfigH264Params.h264VUIParameters);
        display->inputParams.encodeConfigH264Params.h264VUIParameters = NULL;
    }

    LOG_DBG("video_display_Deinit: Output destroy\n");
    if(display->output) {
        NvMediaVideoOutputDestroy(display->output);
    }

    if(display->consumer) {
        NvMediaEglStreamConsumerDestroy(display->consumer);
        display->consumer = NULL;
    }
    LOG_DBG("video_display_Deinit: consumer destroy\n");

    if(display->device) {
        NvMediaDeviceDestroy(display->device);
        display->device = NULL;
    }
}

void video_display_Stop(test_nvmedia_consumer_display_s *display) {
    display->quit = 1;
    if(display->procThread) {
        LOG_DBG("wait for nvmedia video consumer thread exit\n");
        NvThreadDestroy(display->procThread);
    }
}

void video_display_Flush(test_nvmedia_consumer_display_s *display) {
    NvMediaTime timeStamp;
    NvMediaVideoSurface *surface = NULL;
    EGLint streamState = 0;

    do {
        if(!eglQueryStreamKHR(
                display->eglDisplay,
                display->eglStream,
                EGL_STREAM_STATE_KHR,
                &streamState)) {
            LOG_ERR("Nvmedia video consumer, eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
        }

        if(streamState == EGL_STREAM_STATE_DISCONNECTED_KHR) {
            break;
        }

        NvMediaEglStreamConsumerAcquireSurface(display->consumer, &surface, 16, &timeStamp);
        if(surface) {
            LOG_DBG("%s: EGL Consumer: Release surface %p (display->consumer)\n", __func__, surface);
            NvMediaEglStreamConsumerReleaseSurface(display->consumer, surface);
        }
    } while(surface);
}
