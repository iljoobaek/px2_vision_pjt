/*
 * eglstrm_setup.h
 *
 * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

/*
 * DESCRIPTION:   Common EGL stream functions header file
 */

#ifndef _EGLSTRM_SETUP_H_
#define _EGLSTRM_SETUP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "nvmedia_core.h"
#include "nvmedia_surface.h"
#include "nvcommon.h"
#include "cmdline.h"

#define MAX_STRING_SIZE     256
#define SOCK_PATH           "/tmp/nvmedia_egl_socket"


/* -----  Extension pointers  ---------*/
#define EXTENSION_LIST(T) \
    T( PFNEGLQUERYDEVICESEXTPROC,          eglQueryDevicesEXT ) \
    T( PFNEGLQUERYDEVICESTRINGEXTPROC,     eglQueryDeviceStringEXT ) \
    T( PFNEGLGETPLATFORMDISPLAYEXTPROC,    eglGetPlatformDisplayEXT ) \
    T( PFNEGLGETOUTPUTLAYERSEXTPROC,       eglGetOutputLayersEXT ) \
    T( PFNEGLSTREAMCONSUMEROUTPUTEXTPROC,  eglStreamConsumerOutputEXT) \
    T( PFNEGLCREATESTREAMKHRPROC,          eglCreateStreamKHR ) \
    T( PFNEGLDESTROYSTREAMKHRPROC,         eglDestroyStreamKHR ) \
    T( PFNEGLQUERYSTREAMKHRPROC,           eglQueryStreamKHR ) \
    T( PFNEGLQUERYSTREAMU64KHRPROC,        eglQueryStreamu64KHR ) \
    T( PFNEGLQUERYSTREAMTIMEKHRPROC,       eglQueryStreamTimeKHR ) \
    T( PFNEGLSTREAMATTRIBKHRPROC,          eglStreamAttribKHR ) \
    T( PFNEGLCREATESTREAMSYNCNVPROC,       eglCreateStreamSyncNV ) \
    T( PFNEGLCLIENTWAITSYNCKHRPROC,        eglClientWaitSyncKHR ) \
    T( PFNEGLSIGNALSYNCKHRPROC,            eglSignalSyncKHR ) \
    T( PFNEGLSTREAMCONSUMERACQUIREKHRPROC, eglStreamConsumerAcquireKHR ) \
    T( PFNEGLSTREAMCONSUMERRELEASEKHRPROC, eglStreamConsumerReleaseKHR ) \
    T( PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC, \
                        eglStreamConsumerGLTextureExternalKHR ) \
    T( PFNEGLGETSTREAMFILEDESCRIPTORKHRPROC, eglGetStreamFileDescriptorKHR) \
    T( PFNEGLCREATESTREAMFROMFILEDESCRIPTORKHRPROC, eglCreateStreamFromFileDescriptorKHR) \
    T( PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC, eglCreateStreamProducerSurfaceKHR ) \
    T( PFNEGLQUERYSTREAMMETADATANVPROC,    eglQueryStreamMetadataNV ) \
    T( PFNEGLSETSTREAMMETADATANVPROC,      eglSetStreamMetadataNV ) \
    T( PFNEGLSTREAMCONSUMERACQUIREATTRIBEXTPROC, eglStreamConsumerAcquireAttribEXT ) \
    T( PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALATTRIBSNVPROC, \
                        eglStreamConsumerGLTextureExternalAttribsNV )

#define EXTLST_DECL(tx, x)  tx x##fp = NULL;
#define EXTLST_EXTERN(tx, x) extern tx x##fp;
#define EXTLST_ENTRY(tx, x) { (extlst_fnptr_t *)&x##fp, #x },

EGLDisplay EGLDefaultDisplayInit(void);
void EGLDefaultDisplayDeinit(EGLDisplay eglDisplay);


int eglSetupExtensions(void);
EGLStreamKHR EGLStreamInit(EGLDisplay display,
                            TestArgs *args);
void EGLStreamFini(EGLDisplay display,
                        EGLStreamKHR eglStream);


extern NvMediaSurfaceType surfaceType;

#endif
