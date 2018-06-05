/*
 * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

/*
 * DESCRIPTION:   Common egl stream functions
 */

#include "eglstrm_setup.h"
#include "log_utils.h"
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

EXTENSION_LIST(EXTLST_DECL)

typedef void (*extlst_fnptr_t)(void);

static struct {
    extlst_fnptr_t *fnptr;
    char const *name;
} extensionList[] = { EXTENSION_LIST(EXTLST_ENTRY) };

/* Get the required extension function addresses */
static int EGLSetupExtensions(void)
{
    unsigned int i;

    for (i = 0; i < (sizeof(extensionList) / sizeof(*extensionList)); i++) {
        *extensionList[i].fnptr = eglGetProcAddress(extensionList[i].name);
        if (*extensionList[i].fnptr == NULL) {
            LOG_ERR("Couldn't get address of %s()\n", extensionList[i].name);
            return -1;
        }
    }

    return 0;
}

EGLDisplay EGLDefaultDisplayInit(void)
{

    EGLBoolean eglStatus;
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLDeviceEXT devices[16];
    EGLint num_devices = 0;

    LOG_DBG("%s: Enter\n", __func__);

    if (EGLSetupExtensions()) {
        LOG_ERR("%s: failed to setup egl extensions\n", __func__);
        return EGL_NO_DISPLAY;
    }
    eglQueryDevicesEXTfp(5, devices, &num_devices);
    if (num_devices == 0) {
        LOG_ERR("%s: eglQueryDevicesEXT failed\n",__func__);
        return EGL_NO_DISPLAY;
    }

    eglDisplay = eglGetPlatformDisplayEXTfp(EGL_PLATFORM_DEVICE_EXT,
                                          (void*)devices[0], NULL);
    if (EGL_NO_DISPLAY == eglDisplay) {
        LOG_ERR("%s: failed to get default display\n", __func__);
        return EGL_NO_DISPLAY;
    }
    // Initialize EGL
    eglStatus = eglInitialize(eglDisplay, 0, 0);
    if (!eglStatus) {
        LOG_ERR("EGL failed to initialize.\n");
        return EGL_NO_DISPLAY;
    }

    LOG_DBG("%s: Exit\n", __func__);

    return eglDisplay;
}

void EGLDefaultDisplayDeinit(EGLDisplay eglDisplay)
{
    EGLBoolean eglStatus;

    if (EGL_NO_DISPLAY == eglDisplay) {
        return;
    }

    eglStatus = eglTerminate(eglDisplay);
    if (EGL_TRUE != eglStatus) {
        LOG_ERR("%s: Error terminating EGL\n, __func__");
    }
    LOG_DBG("%s: EGL terminated\n", __func__);
}

static int
ConnectProducer(
    TestArgs *args)
{
    int sock;
    struct sockaddr_in server_addr;

    /* This is a dummy message to be sent to consumer by producer
     to verify if connection has been successfully established. */
    const char *data = "Hi, This is a sample producer";
    memset(&server_addr, 0, sizeof(server_addr));

    LOG_DBG("IP: %s port: %d\n", args->ip, args->socketport);

    server_addr.sin_addr.s_addr = inet_addr(args->ip);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(args->socketport);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG_ERR("create socket failed\n");
        return sock;
    }

    while (!connect(sock, (struct sockaddr*)&server_addr,
                sizeof(server_addr))) {
        LOG_DBG("Waiting for consumer\n");
        sleep(2);
    }

    /* To verify successfull connection establishment */
    if ( send(sock , data , strlen(data) , 0) < 0 ) {
        LOG_ERR("Client failed to send data.\n");
        close(sock);
        return -1;
    } else {
        LOG_DBG("Send sucess!\n");
    }

    return sock;
}

static EGLStreamKHR
CreateProducerEGLStream(
                    EGLDisplay display,
                    const EGLint *streamAttr,
                    TestArgs *args)
{
    EGLint streamState = EGL_STREAM_STATE_EMPTY_KHR;
    EGLint attrList[2*MAX_ATTRIB+1]={0};
    int attrIndex = 0, ret = 0;
    EGLStreamKHR eglStream = EGL_NO_STREAM_KHR;

    if ((gsock = ConnectProducer(args)) <= 0) {
        LOG_ERR("Failed to connect to consumer\n");
        return EGL_NO_STREAM_KHR;
    }

    while (streamAttr[attrIndex] != EGL_NONE) {
        attrList[attrIndex] = streamAttr[attrIndex];
        attrIndex++;
    };

    /* set attribute list */
    attrList[attrIndex++] = EGL_CONSUMER_LATENCY_USEC_KHR;
    attrList[attrIndex++] = 0;

    attrList[attrIndex++] = EGL_STREAM_PROTOCOL_NV;
    attrList[attrIndex++] = EGL_STREAM_PROTOCOL_SOCKET_NV;

    attrList[attrIndex++] = EGL_STREAM_TYPE_NV;
    attrList[attrIndex++] = EGL_STREAM_CROSS_PARTITION_NV;
    attrList[attrIndex++] = EGL_SOCKET_TYPE_NV;
    attrList[attrIndex++] = EGL_SOCKET_TYPE_INET_NV;

    attrList[attrIndex++] = EGL_SOCKET_HANDLE_NV;
    attrList[attrIndex++] = gsock;

    attrList[attrIndex++] = EGL_STREAM_ENDPOINT_NV;
    attrList[attrIndex++] = EGL_STREAM_PRODUCER_NV;

    attrList[attrIndex++] = EGL_NONE;

    eglStream = eglCreateStreamKHRfp(display, attrList);
    if (eglStream == EGL_NO_STREAM_KHR) {
        LOG_ERR("producer failed to create eglstream\n");
        return eglStream;
    }
    /* Wait while stream initializes */
    do {
        ret = eglQueryStreamKHRfp(display,
                                eglStream,
                                EGL_STREAM_STATE_KHR,
                                &streamState);

        if (!ret) {
            LOG_ERR("[P] Could not query EGL stream state\n");
            return EGL_NO_STREAM_KHR;
        }
    } while (streamState == EGL_STREAM_STATE_INITIALIZING_NV);

    if (!(streamState == EGL_STREAM_STATE_CREATED_KHR) &&
            !(streamState == EGL_STREAM_STATE_CONNECTING_KHR)) {
        LOG_ERR("EGL stream is not in valid starting state\n");
        return EGL_NO_STREAM_KHR;
    }

    return eglStream;
}

static void
PrintEGLStreamState(
        EGLint streamState)
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

/* Listen on a unix domain socket named <socket_name> and  */
/* receive a file descriptor from another process.         */
/* Returns the file descriptor.  Note: the integer value   */
/* of the file descriptor may be different from the        */
/* integer value in the other process, but the file        */
/* descriptors in each process will refer to the same file */
/* object in the kernel.                                   */
static int
EGLStreamReceivefd(
        const char *socket_name)
{
    int listen_fd;
    struct sockaddr_un sock_addr;
    int connect_fd;
    struct sockaddr_un connect_addr;
    socklen_t connect_addr_len = 0;
    struct msghdr msg;
    struct iovec iov[1];
    char msg_buf[1];
    char ctrl_buf[CMSG_SPACE(sizeof(int))];
    struct cmsghdr *cmsg;
    void *data;
    int recvfd;

    listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        LOG_ERR("EGLStreamReceivefd: socket");
        return -1;
    }
    LOG_DBG("EGLStreamReceivefd: listen_fd: %d\n", listen_fd);

    unlink(socket_name);

    memset(&sock_addr, 0, sizeof(struct sockaddr_un));
    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path,
            socket_name,
            sizeof(sock_addr.sun_path)-1);

    if (bind(listen_fd,
             (const struct sockaddr*)&sock_addr,
             sizeof(struct sockaddr_un))) {
        LOG_ERR("EGLStreamReceivefd: bind");
        return -1;
    }

    if (listen(listen_fd, 1)) {
        LOG_ERR("EGLStreamReceivefd: listen");
        return -1;
    }

    connect_fd = accept(
                    listen_fd,
                    (struct sockaddr *)&connect_addr,
                    &connect_addr_len);
    LOG_DBG("EGLStreamReceivefd: connect_fd: %d\n", connect_fd);
    close(listen_fd);
    unlink(socket_name);
    if (connect_fd < 0) {
        LOG_ERR("EGLStreamReceivefd: accept");
        return -1;
    }

    memset(&msg, 0, sizeof(msg));

    iov[0].iov_base = msg_buf;
    iov[0].iov_len  = sizeof(msg_buf);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    msg.msg_control = ctrl_buf;
    msg.msg_controllen = sizeof(ctrl_buf);

    if (recvmsg(connect_fd, &msg, 0) <= 0) {
        LOG_ERR("EGLStreamReceivefd: recvmsg");
        return -1;
    }

    cmsg = CMSG_FIRSTHDR(&msg);
    if (!cmsg) {
        LOG_DBG("EGLStreamReceivefd: NULL message header\n");
        return -1;
    }
    if (cmsg->cmsg_level != SOL_SOCKET) {
        LOG_DBG("EGLStreamReceivefd: Message level is not SOL_SOCKET\n");
        return -1;
    }
    if (cmsg->cmsg_type != SCM_RIGHTS) {
        LOG_DBG("EGLStreamReceivefd: Message type is not SCM_RIGHTS\n");
        return -1;
    }

    data = CMSG_DATA(cmsg);
    recvfd = *(int *)data;

    return recvfd;
}


EGLStreamKHR
EGLStreamInit(
    EGLDisplay display,
    TestArgs *args)
{
    EGLStreamKHR eglStream=EGL_NO_STREAM_KHR;
#ifdef EGL_NV_stream_metadata
    /*! [docs_eglstream:EGLint]*/
    static const EGLint streamAttrMailboxMode[] = { EGL_METADATA0_SIZE_NV, 16*1024,
                                                    EGL_METADATA1_SIZE_NV, 16*1024,
                                                    EGL_METADATA2_SIZE_NV, 16*1024,
                                                    EGL_METADATA3_SIZE_NV, 16*1024, EGL_NONE };
    /*! [docs_eglstream:EGLint]*/
#else
    static const EGLint streamAttrMailboxMode[] = { EGL_NONE };
#endif /* EGL_NV_stream_metadata */

    EGLint fifo_length = 0, latency = 0, timeout = 0;
    EGLint acquireTimeout = 16000;

    LOG_DBG("EGLStreamInit - Start\n");

    if(args->cross == STANDALONE_CROSS_PART) {
        /* Standalone cross-partition producer */
        EGLint streamState = 0;
        eglStream = CreateProducerEGLStream(display,
                                        streamAttrMailboxMode,
                                        args);
        if (eglStream == EGL_NO_STREAM_KHR) {
            LOG_ERR("EGLStreamInit: Failed to create eglstream\n");
            return 0;
        }
        PrintEGLStreamState(streamState);
    } else {
        /* Standalone cross-process producer */
        EGLNativeFileDescriptorKHR file_descriptor;
        EGLint streamState = 0;

        /* Get the file descriptor of the stream from the consumer process
         * and re-create the EGL stream from it.
         */
        file_descriptor = EGLStreamReceivefd(SOCK_PATH);
        if(file_descriptor == -1) {
            LOG_ERR("EGLStreamInit: Cannot receive EGL file descriptor to socket: %s\n", SOCK_PATH);
            return 0;
        }
        LOG_DBG("Producer file descriptor: %d\n", file_descriptor);
        eglStream = eglCreateStreamFromFileDescriptorKHRfp(
                        display, file_descriptor);
        close(file_descriptor);

        if (eglStream == EGL_NO_STREAM_KHR) {
            LOG_ERR("EGLStreamInit: Couldn't create EGL Stream from fd.\n");
            return 0;
        }
        if(!eglQueryStreamKHRfp(
                display,
                eglStream,
                EGL_STREAM_STATE_KHR,
                &streamState)) {
            LOG_ERR("EGLStreamInit: eglQueryStreamKHR EGL_STREAM_STATE_KHR failed\n");
            return 0;
        }
        PrintEGLStreamState(streamState);
    }

    /* Set stream attribute */
    if(!eglStreamAttribKHRfp(display, eglStream, EGL_CONSUMER_LATENCY_USEC_KHR, 16000)) {
        LOG_ERR("Consumer: eglStreamAttribKHR EGL_CONSUMER_LATENCY_USEC_KHR failed\n");
    }
    if(!eglStreamAttribKHRfp(display, eglStream, EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR,
                                                                    acquireTimeout)) {
        LOG_ERR("Consumer: eglStreamAttribKHR EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR failed\n");
    }

    /* Get stream attributes */
    if(!eglQueryStreamKHRfp(display, eglStream, EGL_STREAM_FIFO_LENGTH_KHR, &fifo_length)) {
        LOG_ERR("Consumer: eglQueryStreamKHR EGL_STREAM_FIFO_LENGTH_KHR failed\n");
    }
    if(!eglQueryStreamKHRfp(display, eglStream, EGL_CONSUMER_LATENCY_USEC_KHR, &latency)) {
        LOG_ERR("Consumer: eglQueryStreamKHR EGL_CONSUMER_LATENCY_USEC_KHR failed\n");
    }
    if(!eglQueryStreamKHRfp(display, eglStream, EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR, &timeout)) {
        LOG_ERR("Consumer: eglQueryStreamKHR EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR failed\n");
    }

    printf("EGL Stream consumer - Mode: Mailbox\n");
    printf("EGL Stream consumer - Latency: %d usec\n", latency);
    printf("EGL Stream consumer - Timeout: %d usec\n", timeout);

    LOG_DBG("EGLStreamInit - End\n");
    return eglStream;
}


void
EGLStreamFini(
        EGLDisplay display,
        EGLStreamKHR eglStream)
{
    /* eglStreamConsumerReleaseKHR(display, eglStream); */
    if ((display != EGL_NO_DISPLAY) && (eglStream != EGL_NO_STREAM_KHR)) {
        eglDestroyStreamKHRfp(display, eglStream);
        LOG_DBG("%s: EGL Stream destroyed\n", __func__);
    }

    if (gsock != -1) {
        close(gsock);
        gsock = -1;
    }

}
