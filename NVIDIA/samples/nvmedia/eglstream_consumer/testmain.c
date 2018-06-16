/*
 * testmain.c
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
// DESCRIPTION:   Simple EGL stream sample app
//
//
#include <eglstrm_setup.h>
#include "nvmedia_core.h"
#include "nvmedia_surface.h"
#include "egl_utils.h"
#include "nvm_consumer.h"
#include "log_utils.h"

#if defined(EXTENSION_LIST)
EXTENSION_LIST(EXTLST_EXTERN)
#endif

NvBool signal_stop = 0;

#define SET_VERSION(x, _major, _minor) \
    x.major = _major; \
    x.minor = _minor;

static void
sig_handler(int sig)
{
    signal_stop = NV_TRUE;
    printf("Signal: %d\n", sig);
}

static NvMediaStatus
CheckVersion(void)
{
    NvMediaVersion version;
    NvMediaStatus status = NVMEDIA_STATUS_OK;

    memset(&version, 0, sizeof(NvMediaVersion));
    status = NvMediaCoreGetVersion(&version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if((version.major != NVMEDIA_CORE_VERSION_MAJOR) ||
       (version.minor != NVMEDIA_CORE_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible core version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_CORE_VERSION_MAJOR, NVMEDIA_CORE_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            version.major, version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&version, 0, sizeof(NvMediaVersion));
    status = NvMediaImageGetVersion(&version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if((version.major != NVMEDIA_IMAGE_VERSION_MAJOR) ||
       (version.minor != NVMEDIA_IMAGE_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible image version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_IMAGE_VERSION_MAJOR, NVMEDIA_IMAGE_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            version.major, version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&version, 0, sizeof(NvMediaVersion));
    status = NvMediaEglStreamGetVersion(&version);
    if(status != NVMEDIA_STATUS_OK)
        return status;

    if((version.major != NVMEDIA_EGLSTREAM_VERSION_MAJOR) ||
       (version.minor != NVMEDIA_EGLSTREAM_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible EGLStream version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_EGLSTREAM_VERSION_MAJOR, NVMEDIA_EGLSTREAM_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            version.major, version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&version, 0, sizeof(NvMediaVersion));
    status = NvMedia2DGetVersion(&version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if ((version.major != NVMEDIA_2D_VERSION_MAJOR) ||
        (version.minor != NVMEDIA_2D_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible 2D version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_2D_VERSION_MAJOR, NVMEDIA_2D_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            version.major, version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&version, 0, sizeof(NvMediaVersion));
    status = NvMediaIDPGetVersion(&version);
    if (status != NVMEDIA_STATUS_OK)
        return status;
    if (version.major != NVMEDIA_IDP_VERSION_MAJOR ||
        version.minor != NVMEDIA_IDP_VERSION_MINOR) {
        LOG_ERR("%s: Incompatible IDP version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
                NVMEDIA_IDP_VERSION_MAJOR, NVMEDIA_IDP_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
                version.major, version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    return status;
}

int main(int argc, char **argv)
{
    int         failure = 1;
    EglUtilState *eglUtil = NULL;
    EGLStreamKHR eglStream=EGL_NO_STREAM_KHR;
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    volatile NvBool producerDone = 0;
    volatile NvBool consumerDone = 0;

    test_nvmedia_consumer_display_s video_display;

    memset(&video_display, 0, sizeof(test_nvmedia_consumer_display_s));

    // Hook up Ctrl-C handler
    signal(SIGINT, sig_handler);

    if(CheckVersion() != NVMEDIA_STATUS_OK) {
        return failure;
    }

    EglUtilOptions options;
    memset(&options, 0, sizeof(EglUtilOptions));

	eglUtil = EGLUtilInit(&options);
	if (!eglUtil) {
		LOG_ERR("main - failed to initialize egl \n");
		goto done;
	}
	eglDisplay = eglUtil->display;

    eglStream = EGLStreamInit(eglDisplay);
    if(!eglStream) {
        goto done;
    }

	//Init consumer
	LOG_DBG("main - image_display_init\n");
	if(!image_display_init(&consumerDone, &video_display, eglDisplay, eglStream)) {
		LOG_ERR("main: image Display init failed\n");
		goto done;
	}

    // wait for signal_stop or producer done
    while(!signal_stop && !producerDone && !consumerDone) {
        usleep(1000);
    }

done:
	EGLStreamFini(eglDisplay, eglStream);
	if(eglUtil) {
		EGLUtilDeinit(eglUtil);
	}
	LOG_DBG("main - clean up done\n");

    return failure;
}
