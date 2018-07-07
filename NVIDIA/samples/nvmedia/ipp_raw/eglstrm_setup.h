/*
 * Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef _NVMEDIA_EGLSTRM_SETUP_H_
#define _NVMEDIA_EGLSTRM_SETUP_H_

#include <stdio.h>

#include "nvmedia_eglstream.h"
#include "nvmedia_ipp.h"
#include "egl_utils.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifdef __cplusplus
extern "C" {
#endif

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

/* struct to give client params of the connection */
/* struct members are read-only to client */
typedef struct _EglStreamClient {
    EGLDisplay   display;
    EGLStreamKHR eglStream[NVMEDIA_MAX_AGGREGATE_IMAGES];
    NvBool       fifoMode;
    NvU32        numofStream;
} EglStreamClient;

EglStreamClient*
EGLStreamInit(EGLDisplay display,
                        NvU32 numOfStreams,
                        NvBool fifoMode);
NvMediaStatus
EGLStreamFini(EglStreamClient *client);

#ifdef __cplusplus
}
#endif

#endif /* _NVMEDIA_EGLSTRM_SETUP_H_*/
