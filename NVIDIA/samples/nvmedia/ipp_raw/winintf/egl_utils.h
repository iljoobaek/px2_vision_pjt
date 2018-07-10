/*
 * egl_utils.h
 *
 * Copyright (c) 2015-2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef _TEST_EGL_SETUP_H
#define _TEST_EGL_SETUP_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2ext_nv.h>
#include "nvcommon.h"
#include "nvmedia_core.h"
#include "nvmedia_surface.h"

/* -----  Extension pointers  ---------*/
typedef struct {
    int windowSize[2];                      // Window size
    int windowOffset[2];                    // Window offset
    int displayId;
    int windowId;
    NvBool vidConsumer;
} EglUtilOptions;

typedef struct _EglUtilState {
    EGLDisplay              display;
    EGLSurface              surface;
    EGLConfig               config;
    EGLContext              context;
    EGLint                  width;
    EGLint                  height;
    EGLint                  xoffset;
    EGLint                  yoffset;
    EGLint                  displayId;
    EGLint                  windowId;
    NvBool                  vidConsumer;
    EGLDisplay              display_dGPU;
    EGLContext              context_dGPU;
} EglUtilState;

int EGLUtilCreateContext(EglUtilState *state);

EGLDisplay EGLUtilDefaultDisplayInit(void);
EglUtilState *EGLUtilInit(EglUtilOptions *);
void EGLUtilDefaultDisplayDeinit(EGLDisplay eglDisplay);
void EGLUtilDeinit(EglUtilState *state);
void EGLUtilDestroyContext(EglUtilState *state);
int EGLUtilInit_dGPU(EglUtilState *state);
int EGLUtilCreateContext_dGPU(EglUtilState *state);

NvBool WindowSystemInit(EglUtilState *state);
void WindowSystemTerminate(void);
int WindowSystemWindowInit(EglUtilState *state);
void WindowSystemWindowTerminate(EglUtilState *state);
NvBool WindowSystemEglSurfaceCreate(EglUtilState *state);
NvBool WindowSystemInit_dGPU(EglUtilState *state);
void WindowSystemTerminate_dGPU(void);
#endif /* _TEST_EGL_SETUP_H */
