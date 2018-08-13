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

extern "C" {

//int getImgBuffer(unsigned char** buf1, unsigned char** buf2, unsigned char** buf3, unsigned char** buf4);
int getImgBuffer(unsigned char** buf1, unsigned char** buf2, unsigned char** buf3, unsigned char** buf4);
int eglconsumer_main();

}

#endif //#define _EGLCONSUMER_H_
