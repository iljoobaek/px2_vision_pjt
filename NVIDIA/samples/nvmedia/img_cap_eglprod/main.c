/* Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <signal.h>

#include "main.h"
#include "check_version.h"
#include "capture.h"
#include "save.h"
#include "composite.h"
#include "err_handler.h"
#include "nvmimg_producer.h"

/* Quit flag. Out of context structure for sig handling */
static volatile uint32_t *quit_flag;

static void
SigHandler(
        int signum)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGKILL, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    *quit_flag = NVMEDIA_TRUE;

    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGKILL, SIG_DFL);
    signal(SIGSTOP, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
}

static void
SigSetup(
        void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = SigHandler;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGKILL, &action, NULL);
    sigaction(SIGSTOP, &action, NULL);
    sigaction(SIGHUP, &action, NULL);
}

static int
ExecuteNextCommand(
        NvMainContext *ctx) {
    char input[256] = { 0 };

    if (!fgets(input, 256, stdin)) {
        LOG_ERR("%s: Failed to read commnad\n", __func__);
        return -1;
    }

    /* Remove new line character */
    if (input[strlen(input) - 1] == '\n')
        input[strlen(input) - 1] = '\0';


    if (!strcasecmp(input, "q") || !strcasecmp(input, "quit")) {
        *quit_flag = NVMEDIA_TRUE;
        return 0;
    } else if (!strcasecmp(input, "h") || !strcasecmp(input, "help")) {
        PrintUsage();
        return 0;
    } else if (!strcasecmp(input, "")) {
        return 0;
    }

    return 0;
}


int
main(
    int argc,
    char *argv[])
{
    TestArgs allArgs;
    NvMainContext mainCtx;

    memset(&allArgs, 0, sizeof(TestArgs));
    memset(&mainCtx, 0 , sizeof(NvMainContext));

    if (CheckModulesVersion() != NVMEDIA_STATUS_OK) {
        return -1;
    }

    if (IsFailed(ParseArgs(argc, argv, &allArgs))) {
        return -1;
    }

    quit_flag = &mainCtx.quit;
    SigSetup();

    /* Initialize context */
    mainCtx.testArgs = &allArgs;

    /* Initialize all the components */
    if (EglProducerInit(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to Initialize Producer\n", __func__);
        goto done;
    }

    if (CaptureInit(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to Initialize Capture\n", __func__);
        goto done;
    }

    if (SaveInit(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to Initialize Save\n", __func__);
        goto done;
    }

    if (CompositeInit(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to Initialize Composite\n", __func__);
        goto done;
    }

    if (ErrHandlerInit(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to Initialize ErrHandler\n", __func__);
        goto done;
    }

    /* Call Proc for each component */
    if (EglProducerProc(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: ProducerProc Failed\n", __func__);
        goto done;
    }

	//multiple eglstream
	if (EglProducerInit(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to Initialize Producer\n", __func__);
        goto done;
    }
	 if (EglProducerProc(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: ProducerProc Failed\n", __func__);
        goto done;
    }
	//end

    if (CaptureProc(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: CaptureProc Failed\n", __func__);
        goto done;
    }

    if (SaveProc(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: SaveProc Failed\n", __func__);
        goto done;
    }

    if (CompositeProc(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: CompositeProc Failed\n", __func__);
        goto done;
    }

    if (ErrHandlerProc(&mainCtx) != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: ErHandlerProc Failed\n", __func__);
        goto done;
    }

    LOG_MSG("Type \"h\" for a list of options\n");

    while (!mainCtx.quit) {
        if (!allArgs.numFrames.isUsed) {
            LOG_MSG("-");
            ExecuteNextCommand(&mainCtx);
        }
    }

done:
    EglProducerFini(&mainCtx);
    CompositeFini(&mainCtx);
    SaveFini(&mainCtx);
    CaptureFini(&mainCtx);
    ErrHandlerFini(&mainCtx);
    return 0;
}
