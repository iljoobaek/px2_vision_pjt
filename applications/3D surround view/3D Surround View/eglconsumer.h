/*
 * nvmimg_consumer.h
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
// DESCRIPTION:   Simple nvmedia consumer header file
//

#ifndef _EGLCONSUMER_H_
#define _EGLCONSUMER_H_

#ifdef __cplusplus
extern "C" {
#endif

int getImgBuffer(unsigned char* inputBuffer);
int eglconsumer_main();

#ifdef __cplusplus
}
#endif

#endif //#define _EGLCONSUMER_H_
