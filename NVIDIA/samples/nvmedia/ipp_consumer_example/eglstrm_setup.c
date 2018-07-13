/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

//
// DESCRIPTION:   Common egl stream functions
//

#include "eglstrm_setup.h"
#include "log_utils.h"
#include "egl_utils.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <EGL/eglext.h>

#define MAX_ATTRIB    (31)
static int gsock = -1;

EXTENSION_LIST(EXTLST_EXTERN)
#ifndef NVMEDIA_GHSI
/* Send <fd_to_send> (a file descriptor) to another process */
/* over a unix domain socket named <socket_name>.           */
/* <socket_name> can be any nonexistant filename.           */
static int EGLStreamSendfd(const char *socket_name, int fd_to_send)
{
    int sock_fd;
    struct sockaddr_un sock_addr;
    struct msghdr msg;
    struct iovec iov[1];
    char ctrl_buf[CMSG_SPACE(sizeof(int))];
    struct cmsghdr *cmsg = NULL;
    void *data;
    int res;
    int wait_loop = 0;

    sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(sock_fd < 0) {
        LOG_ERR("EGLStreamSendfd: socket");
        return -1;
    }
    LOG_DBG("EGLStreamSendfd: sock_fd: %d\n", sock_fd);

    memset(&sock_addr, 0, sizeof(struct sockaddr_un));
    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path,
            socket_name,
            sizeof(sock_addr.sun_path)-1);

    while(connect(sock_fd,
                (const struct sockaddr*)&sock_addr,
                sizeof(struct sockaddr_un))) {
        if(wait_loop < 60) {
            if(!wait_loop)
                printf("Waiting for EGL stream producer ");
            else
                printf(".");
            fflush(stdout);
            sleep(1);
            wait_loop++;
        } else {
            printf("\nWaiting timed out\n");
            return -1;
        }
    }
    if(wait_loop)
        printf("\n");

    LOG_DBG("EGLStreamSendfd: Wait is done\n");

    memset(&msg, 0, sizeof(msg));

    iov[0].iov_len  = 1;    // must send at least 1 byte
    iov[0].iov_base = "x";  // any byte value (value ignored)
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    memset(ctrl_buf, 0, sizeof(ctrl_buf));
    msg.msg_control = ctrl_buf;
    msg.msg_controllen = sizeof(ctrl_buf);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    data = CMSG_DATA(cmsg);
    *(int *)data = fd_to_send;

    msg.msg_controllen = cmsg->cmsg_len;

    res = sendmsg(sock_fd, &msg, 0);
    LOG_DBG("EGLStreamSendfd: sendmsg: res: %d\n", res);

    if(res <= 0) {
        LOG_ERR("EGLStreamSendfd: sendmsg");
        return -1;
    }

    close(sock_fd);

    return 0;
}


#endif

void PrintEGLStreamState(EGLint streamState)
{
    #define STRING_VAL(x) {""#x"", x}
    struct {
        char *name;
        EGLint val;
    } EGLState[9] = {
        STRING_VAL(EGL_STREAM_STATE_CREATED_KHR),
        STRING_VAL(EGL_STREAM_STATE_CONNECTING_KHR),
        STRING_VAL(EGL_STREAM_STATE_EMPTY_KHR),
        STRING_VAL(EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR),
        STRING_VAL(EGL_STREAM_STATE_OLD_FRAME_AVAILABLE_KHR),
        STRING_VAL(EGL_STREAM_STATE_DISCONNECTED_KHR),
        STRING_VAL(EGL_BAD_STREAM_KHR),
        STRING_VAL(EGL_BAD_STATE_KHR),
        { NULL, 0 }
    };
    int i = 0;

    while(EGLState[i].name) {
        if(streamState == EGLState[i].val) {
            printf("%s\n", EGLState[i].name);
            return;
        }
        i++;
    }
    printf("Invalid %d\n", streamState);
}

EGLStreamKHR EGLStreamInit(EGLDisplay display)
{
    EGLStreamKHR eglStream=EGL_NO_STREAM_KHR;
#ifdef EGL_NV_stream_metadata
    //! [docs_eglstream:EGLint]
    static const EGLint streamAttrMailboxMode[] = { EGL_METADATA0_SIZE_NV, 16*1024,
                                                    EGL_METADATA1_SIZE_NV, 16*1024,
                                                    EGL_METADATA2_SIZE_NV, 16*1024,
                                                    EGL_METADATA3_SIZE_NV, 16*1024, EGL_NONE };
    //static const EGLint streamAttrFIFOMode[] = { EGL_STREAM_FIFO_LENGTH_KHR, 4,
    //                                             EGL_METADATA0_SIZE_NV, 16*1024,
    //                                             EGL_METADATA1_SIZE_NV, 16*1024,
    //                                             EGL_METADATA2_SIZE_NV, 16*1024,
    //                                             EGL_METADATA3_SIZE_NV, 16*1024, EGL_NONE };
    //! [docs_eglstream:EGLint]
#else
    static const EGLint streamAttrMailboxMode[] = { EGL_NONE };
    static const EGLint streamAttrFIFOMode[] = { EGL_STREAM_FIFO_LENGTH_KHR, 4, EGL_NONE };
#endif //EGL_NV_stream_metadata

    EGLint fifo_length = 0, latency = 0, timeout = 0;
    GLint acquireTimeout = 16000;

	LOG_DBG("EGLStreamInit - Start\n");
	//! [docs_eglstream:eglCreateStreamKHR]
	// Standalone consumer or no standalone mode
	eglStream = eglCreateStreamKHR(display,streamAttrMailboxMode);
	if (eglStream == EGL_NO_STREAM_KHR) {
		LOG_ERR("EGLStreamInit: Couldn't create eglStream.\n");
		return 0;
	}
	//! [docs_eglstream:eglCreateStreamKHR]

	EGLNativeFileDescriptorKHR file_descriptor;
	int res;

	// In standalone consumer case get the file descriptor for the EGL stream
	// send to the procucer process.
	file_descriptor = eglGetStreamFileDescriptorKHR(display, eglStream);
	if(file_descriptor == EGL_NO_FILE_DESCRIPTOR_KHR) {
		LOG_ERR("EGLStreamInit: Cannot get EGL file descriptor\n");
		return 0;
	}
	LOG_DBG("Consumer file descriptor: %d\n", file_descriptor);
	res = EGLStreamSendfd(SOCK_PATH, file_descriptor);
	if(res == -1) {
		LOG_ERR("EGLStreamInit: Cannot send EGL file descriptor to socket: %s\n", SOCK_PATH);
		return 0;
	}
	close(file_descriptor);

	// Set stream attribute
    if(!eglStreamAttribKHR(display, eglStream, EGL_CONSUMER_LATENCY_USEC_KHR, 16000)) {
        LOG_ERR("Consumer: eglStreamAttribKHR EGL_CONSUMER_LATENCY_USEC_KHR failed\n");
    }
    if(!eglStreamAttribKHR(display, eglStream, EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR, acquireTimeout)) {
        LOG_ERR("Consumer: eglStreamAttribKHR EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR failed\n");
    }

    // Get stream attributes
    if(!eglQueryStreamKHR(display, eglStream, EGL_STREAM_FIFO_LENGTH_KHR, &fifo_length)) {
        LOG_ERR("Consumer: eglQueryStreamKHR EGL_STREAM_FIFO_LENGTH_KHR failed\n");
    }
    if(!eglQueryStreamKHR(display, eglStream, EGL_CONSUMER_LATENCY_USEC_KHR, &latency)) {
        LOG_ERR("Consumer: eglQueryStreamKHR EGL_CONSUMER_LATENCY_USEC_KHR failed\n");
    }
    if(!eglQueryStreamKHR(display, eglStream, EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR, &timeout)) {
        LOG_ERR("Consumer: eglQueryStreamKHR EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR failed\n");
    }

	printf("EGL Stream consumer - Mode: Mailbox\n");
	printf("EGL Stream consumer - Latency: %d usec\n", latency);
    printf("EGL Stream consumer - Timeout: %d usec\n", timeout);

    LOG_DBG("EGLStreamInit - End\n");
    return eglStream;
}


void EGLStreamFini(EGLDisplay display,EGLStreamKHR eglStream)
{
    //eglStreamConsumerReleaseKHR(display, eglStream);
    if ((display != EGL_NO_DISPLAY) && (eglStream != EGL_NO_STREAM_KHR)) {
        eglDestroyStreamKHR(display, eglStream);
        LOG_DBG("%s: EGL Stream destroyed\n", __func__);
    }

    if (gsock != -1) {
        close(gsock);
        gsock = -1;
    }

}
