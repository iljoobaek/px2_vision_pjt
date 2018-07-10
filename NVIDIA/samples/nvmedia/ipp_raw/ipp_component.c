/*
 * Copyright (c) 2015-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "ipp_component.h"
#include "sample_plugin.h"
#include "nvmedia_acp.h"

#define BUFFER_POOLS_COUNT 3

static NvMediaStatus
IPPSetICPBufferPoolConfig (
    IPPCtx *ctx,
    NvMediaIPPBufferPoolParamsNew *poolConfig)
{
    memset(poolConfig, 0, sizeof(NvMediaIPPBufferPoolParamsNew));

    poolConfig->width = ctx->inputWidth;
    poolConfig->height = ctx->inputHeight;
    poolConfig->portType = NVMEDIA_IPP_PORT_IMAGE_1;
    poolConfig->poolBuffersNum = IMAGE_BUFFERS_POOL_SIZE;
    poolConfig->surfaceType = ctx->inputSurfType;
    poolConfig->imagesCount = ctx->imagesNum;

    /* Set SurfaceAllocAttrs */
    poolConfig->surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
    poolConfig->surfAllocAttrs[0].value = poolConfig->width;
    poolConfig->surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
    poolConfig->surfAllocAttrs[1].value = poolConfig->height;
    poolConfig->surfAllocAttrs[2].type = NVM_SURF_ATTR_EMB_LINES_TOP;
    poolConfig->surfAllocAttrs[2].value = ctx->embeddedDataLinesTop;
    poolConfig->surfAllocAttrs[3].type = NVM_SURF_ATTR_EMB_LINES_BOTTOM;
    poolConfig->surfAllocAttrs[3].value = ctx->embeddedDataLinesBottom;
    poolConfig->surfAllocAttrs[4].type = NVM_SURF_ATTR_CPU_ACCESS;
    poolConfig->surfAllocAttrs[4].value = NVM_SURF_ATTR_CPU_ACCESS_CACHED;
    poolConfig->surfAllocAttrs[5].type = NVM_SURF_ATTR_ALLOC_TYPE;
    poolConfig->surfAllocAttrs[5].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
    poolConfig->numSurfAllocAttrs = 6;

    return NVMEDIA_STATUS_OK;
}

static NvMediaStatus
IPPSetISPBufferPoolConfig (
    IPPCtx *ctx,
    NvMediaIPPBufferPoolParamsNew *poolConfig,
    NvMediaIPPBufferPoolParamsNew *poolStatsConfig)
{
    char *env;
    memset(poolConfig, 0, sizeof(NvMediaIPPBufferPoolParamsNew));
    poolConfig->portType = NVMEDIA_IPP_PORT_IMAGE_1;
    poolConfig->poolBuffersNum = IMAGE_BUFFERS_POOL_SIZE;
    poolConfig->height = ctx->inputHeight;
    poolConfig->surfaceType = ctx->ispOutType;

    poolConfig->imagesCount = 1;
    poolConfig->width = ctx->inputWidth;
    /* Set SurfaceAllocAttrs */
    poolConfig->surfAllocAttrs[0].type = NVM_SURF_ATTR_WIDTH;
    poolConfig->surfAllocAttrs[0].value = poolConfig->width;
    poolConfig->surfAllocAttrs[1].type = NVM_SURF_ATTR_HEIGHT;
    poolConfig->surfAllocAttrs[1].value = poolConfig->height;
    poolConfig->surfAllocAttrs[2].type = NVM_SURF_ATTR_CPU_ACCESS;
    poolConfig->surfAllocAttrs[2].value = NVM_SURF_ATTR_CPU_ACCESS_CACHED;
    poolConfig->surfAllocAttrs[3].type = NVM_SURF_ATTR_ALLOC_TYPE;
    poolConfig->surfAllocAttrs[3].value = NVM_SURF_ATTR_ALLOC_ISOCHRONOUS;
    poolConfig->numSurfAllocAttrs = 4;
    if ((env = getenv("DISPLAY_VM"))) {
        poolConfig->surfAllocAttrs[4].type = NVM_SURF_ATTR_PEER_VM_ID;
        poolConfig->surfAllocAttrs[4].value = atoi(env);
        poolConfig->numSurfAllocAttrs += 1;
    }

    // Configure statistics port
    memset(poolStatsConfig, 0, sizeof(NvMediaIPPBufferPoolParamsNew));
    poolStatsConfig->portType = NVMEDIA_IPP_PORT_STATS_1;
    poolStatsConfig->poolBuffersNum = STATS_BUFFERS_POOL_SIZE;

    return NVMEDIA_STATUS_OK;
}

static NvMediaStatus
IPPSetControlAlgorithmBufferPoolConfig (
    IPPCtx *ctx,
    NvMediaIPPBufferPoolParamsNew *poolConfig)
{
    memset(poolConfig, 0, sizeof(NvMediaIPPBufferPoolParamsNew));
    poolConfig->portType = NVMEDIA_IPP_PORT_SENSOR_CONTROL_1;
    poolConfig->poolBuffersNum = SENSOR_BUFFERS_POOL_SIZE;

    return NVMEDIA_STATUS_OK;
}

NvMediaStatus
IPPSetCaptureSettings (
    IPPCtx *ctx,
    CaptureConfigParams *config)
{
    NvMediaICPSettings *settings = &ctx->captureSettings;
    NvMediaICPInputFormat *inputFormat = &settings->inputFormat;
    ExtImgDevProperty *extImgDevProperty = &ctx->extImgDevice->property;

    if(!strcasecmp(config->interface, "csi-a"))
        settings->interfaceType = NVMEDIA_IMAGE_CAPTURE_CSI_INTERFACE_TYPE_CSI_A;
    else if(!strcasecmp(config->interface, "csi-b"))
        settings->interfaceType = NVMEDIA_IMAGE_CAPTURE_CSI_INTERFACE_TYPE_CSI_B;
    else if(!strcasecmp(config->interface, "csi-ab"))
        settings->interfaceType = NVMEDIA_IMAGE_CAPTURE_CSI_INTERFACE_TYPE_CSI_AB;
    else if(!strcasecmp(config->interface, "csi-c"))
        settings->interfaceType = NVMEDIA_IMAGE_CAPTURE_CSI_INTERFACE_TYPE_CSI_C;
    else if(!strcasecmp(config->interface, "csi-d"))
        settings->interfaceType = NVMEDIA_IMAGE_CAPTURE_CSI_INTERFACE_TYPE_CSI_D;
    else if(!strcasecmp(config->interface, "csi-cd"))
        settings->interfaceType = NVMEDIA_IMAGE_CAPTURE_CSI_INTERFACE_TYPE_CSI_CD;
    else if(!strcasecmp(config->interface, "csi-e"))
        settings->interfaceType = NVMEDIA_IMAGE_CAPTURE_CSI_INTERFACE_TYPE_CSI_E;
    else if(!strcasecmp(config->interface, "csi-f"))
        settings->interfaceType = NVMEDIA_IMAGE_CAPTURE_CSI_INTERFACE_TYPE_CSI_F;
    else if(!strcasecmp(config->interface, "csi-ef"))
        settings->interfaceType = NVMEDIA_IMAGE_CAPTURE_CSI_INTERFACE_TYPE_CSI_EF;
    else {
        LOG_ERR("%s: Bad interface-type specified: %s.Using csi-ab as default\n",
                __func__,
                config->interface);
        settings->interfaceType = NVMEDIA_IMAGE_CAPTURE_CSI_INTERFACE_TYPE_CSI_AB;
    }

    NVM_SURF_FMT_DEFINE_ATTR(surfFormatAttrs);
    if (!strcasecmp(config->surfaceFormat, "raw12")) {
        NVM_SURF_FMT_SET_ATTR_RAW(surfFormatAttrs,RGGB,INT,12,PL);
        ctx->rawBytesPerPixel = 2;
    } else {
        LOG_WARN("Bad CSI capture surface format: %s. Using rgb as default\n",
                 config->surfaceFormat);
        NVM_SURF_FMT_SET_ATTR_RGBA(surfFormatAttrs,RGBA,UINT,8,PL);
    }

    surfFormatAttrs[NVM_SURF_ATTR_COMPONENT_ORDER].value += extImgDevProperty->pixelOrder;
    ctx->inputSurfType = NvMediaSurfaceFormatGetType(surfFormatAttrs, NVM_SURF_FMT_ATTR_MAX);
    settings->surfaceType = ctx->inputSurfType;

    inputFormat->inputFormatType = extImgDevProperty->inputFormatType;
    inputFormat->bitsPerPixel = extImgDevProperty->bitsPerPixel;
    ctx->embeddedDataLinesTop = config->embeddedDataLinesTop;
    ctx->embeddedDataLinesBottom = config->embeddedDataLinesBottom;
    ctx->bitsPerPixel = extImgDevProperty->bitsPerPixel;
    ctx->pixelOrder = extImgDevProperty->pixelOrder;
    LOG_DBG("Embedded data lines top: %u\nEmbedded data lines bottom: %u\n",
            config->embeddedDataLinesTop, config->embeddedDataLinesBottom);

    LOG_DBG("%s: Setting capture parameters\n", __func__);
    if(sscanf(config->resolution,
              "%hux%hu",
              &settings->width,
              &settings->height) != 2) {
        LOG_ERR("%s: Bad resolution: %s\n", __func__, config->resolution);
        return NVMEDIA_STATUS_ERROR;
    }

    if((ctx->imagesNum > 1) && !ctx->useVirtualChannels)
        settings->width *= ctx->imagesNum;

    settings->startX = 0;
    settings->startY = 0;
    settings->embeddedDataType = extImgDevProperty->embDataType;
    settings->embeddedDataLines = config->embeddedDataLinesTop + config->embeddedDataLinesBottom;
    settings->interfaceLanes = config->csiLanes;
    /* pixel frequency is from imgDevPropery, it is calculated by (VTS * HTS * FrameRate) * n sensors */
    settings->pixelFrequency = extImgDevProperty->pixelFrequency;

    LOG_INFO("Capture params:\nInterface type: %u\nStart X,Y: %u,%u\n",
             settings->interfaceType, settings->startX, settings->startY);
    LOG_INFO("resolution: %ux%u\nextra-lines: %u\ninterface-lanes: %u\n\n",
             settings->width, settings->height, settings->embeddedDataLines, settings->interfaceLanes);

    return NVMEDIA_STATUS_OK;
}

// Create Sensor Control Component
static NvMediaStatus IPPCreateSensorControlComponent(IPPCtx *ctx)
{
    NvU32 i;
    NvMediaIPPIscComponentConfig iscComponentConfig;

    memset(&iscComponentConfig, 0, sizeof(NvMediaIPPIscComponentConfig));
    for (i = 0; i < ctx->ippNum; i++) {
        iscComponentConfig.iscSensorDevice = ctx->extImgDevice->iscSensor[i];

        ctx->ippIscComponents[i] = NvMediaIPPComponentCreateNew(ctx->ipp[i],     //ippPipeline
                                      NVMEDIA_IPP_COMPONENT_ISC, //componentType
                                      NULL,                                 //bufferPools
                                      &iscComponentConfig);                 //componentConfig
        if (!ctx->ippIscComponents[i]) {
            LOG_ERR("%s: Failed to create sensor ISC component\n", __func__);
            goto failed;
        }
    }

    LOG_DBG("%s: NvMediaIPPComponentCreateNew ISC\n", __func__);

    return NVMEDIA_STATUS_OK;
failed:
    return NVMEDIA_STATUS_ERROR;
}

// Create CaptureEx Component
static NvMediaStatus IPPCreateCaptureComponentEx(IPPCtx *ctx)
{
    NvMediaStatus status;
    NvMediaICPSettingsEx icpSettingsEx;
    NvMediaIPPBufferPoolParamsNew *bufferPools[4 + 1], bufferPool;
    NvU32 i;

    IPPSetICPBufferPoolConfig(ctx, &bufferPool);
    memset(&icpSettingsEx, 0, sizeof(NvMediaICPSettingsEx));

    icpSettingsEx.interfaceType  = ctx->captureSettings.interfaceType;
    icpSettingsEx.interfaceLanes = ctx->captureSettings.interfaceLanes;
    icpSettingsEx.numVirtualGroups = ctx->ippNum;

    for (i = 0; i < ctx->imagesNum; i++) {
        icpSettingsEx.virtualGroups[i].numVirtualChannels = 1;
        icpSettingsEx.virtualGroups[i].virtualChannels[0].virtualChannelIndex = ctx->extImgDevice->property.vcId[i];
        memcpy(&icpSettingsEx.virtualGroups[i].virtualChannels[0].icpSettings, &ctx->captureSettings,
               sizeof(NvMediaICPSettings));
        bufferPools[i] = &bufferPool;
    }
    bufferPools[ctx->imagesNum] = NULL;

    ctx->ippComponents[0][0] =
        NvMediaIPPComponentCreateNew(ctx->ipp[0],                      //ippPipeline
                                  NVMEDIA_IPP_COMPONENT_ICP_EX, //componentType
                                  bufferPools,                      //bufferPools
                                  &icpSettingsEx);                  //componentConfig
    if (!ctx->ippComponents[0][0]) {
        LOG_ERR("%s: Failed to create image capture component\n", __func__);
        return NVMEDIA_STATUS_ERROR;
    }

    ctx->componentNum[0]++;

    // Put ICP as first component in all other pipelines
    for (i = 1; i< ctx->ippNum; i++) {
        status = NvMediaIPPComponentAddToPipeline(ctx->ipp[i], ctx->ippComponents[0][0]);
        if (IsFailed(status)) {
            LOG_ERR("%s: Failed to add image capture component to IPP %d\n", __func__, i);
            return status;
        }
        ctx->ippComponents[i][0] = ctx->ippComponents[0][0];
        ctx->componentNum[i]++;
    }

    return NVMEDIA_STATUS_OK;
}

// Create Capture Component
static NvMediaStatus IPPCreateCaptureComponent(IPPCtx *ctx)
{
    NvMediaIPPIcpComponentConfig icpConfig;
    NvMediaIPPBufferPoolParamsNew *bufferPools[BUFFER_POOLS_COUNT + 1], bufferPool;
    // Create Capture component
    bufferPools[0] = &bufferPool;
    bufferPools[1] = NULL;
    IPPSetICPBufferPoolConfig(ctx, &bufferPool);
    NvU32 i;

    memset(&icpConfig, 0, sizeof(NvMediaIPPIcpComponentConfig));
    icpConfig.settings = &ctx->captureSettings;
    icpConfig.siblingsNum = (ctx->imagesNum > 1) ? ctx->imagesNum : 0;

    ctx->ippComponents[0][0] =
            NvMediaIPPComponentCreateNew(ctx->ipp[0],                   //ippPipeline
                                      NVMEDIA_IPP_COMPONENT_ICP,    //componentType
                                      bufferPools,                     //bufferPools
                                      &icpConfig);                      //componentConfig
    if (!ctx->ippComponents[0][0]) {
        LOG_ERR("%s: Failed to create image capture component\n", __func__);
        goto failed;
    }
    LOG_DBG("%s: NvMediaIPPComponentCreateNew capture\n", __func__);

    ctx->componentNum[0]++;
    // Put ICP as first component in all other pipelines
    for (i=1; i<ctx->ippNum; i++) {
        if (IsFailed(NvMediaIPPComponentAddToPipeline(ctx->ipp[i], ctx->ippComponents[0][0]))) {
            LOG_ERR("%s: Failed to add image capture component to IPP %d\n", __func__, i);
            goto failed;
        }
        ctx->ippComponents[i][0] = ctx->ippComponents[0][0];
        ctx->componentNum[i]++;
    }

    return NVMEDIA_STATUS_OK;
failed:
    return NVMEDIA_STATUS_ERROR;
}

// Create ISP Component
static NvMediaStatus IPPCreateISPComponent(IPPCtx *ctx)
{
    NvMediaIPPIspComponentConfig ispConfig;
    NvMediaIPPBufferPoolParamsNew *bufferPools[BUFFER_POOLS_COUNT + 1], bufferPool, bufferPool2;
    NvU32 i;

    for (i=0; i<ctx->ippNum; i++) {
        if (ctx->ispEnabled[i]) {
            memset(&ispConfig, 0, sizeof(NvMediaIPPIspComponentConfig));
            ispConfig.ispSelect = (i<2) ? NVMEDIA_ISP_SELECT_ISP_A : NVMEDIA_ISP_SELECT_ISP_B;

            bufferPools[0] = &bufferPool;
            bufferPools[1] = &bufferPool2;
            bufferPools[2] = NULL;
            IPPSetISPBufferPoolConfig(ctx, &bufferPool, &bufferPool2);

            ctx->ippComponents[i][ctx->componentNum[i]] = ctx->ippIspComponents[i] =
                    NvMediaIPPComponentCreateNew(ctx->ipp[i],               //ippPipeline
                                              NVMEDIA_IPP_COMPONENT_ISP,    //componentType
                                              bufferPools,                  //bufferPools
                                              &ispConfig);                  //componentConfig
            if (!ctx->ippComponents[i][ctx->componentNum[i]]) {
                LOG_ERR("%s: Failed to create image ISP component for pipeline %d\n", __func__, i);
                goto failed;
            }
            LOG_DBG("%s: NvMediaIPPComponentCreateNew ISP \n", __func__);
            ctx->componentNum[i]++;
        }
    }

    return NVMEDIA_STATUS_OK;
failed:
    return NVMEDIA_STATUS_ERROR;
}

// Create Control Algorithm Component
static NvMediaStatus IPPCreateControlAlgorithmComponent(IPPCtx *ctx)
{
    NvMediaIPPControlAlgorithmComponentConfig controlAlgorithmConfig;
    NvU32 i;
    NvMediaIPPBufferPoolParamsNew *bufferPools[BUFFER_POOLS_COUNT + 1], bufferPool;
    NvMediaIPPPluginFuncs nvpluginFuncs = {
        .createFunc = &NvMediaACPCreate,
        .destroyFunc = &NvMediaACPDestroy,
        .processExFunc = &NvMediaACPProcess
    };
    NvMediaIPPPluginFuncs samplepluginFuncs = {
        .createFunc = &NvSampleACPCreate,
        .destroyFunc = &NvSampleACPDestroy,
        .processExFunc = &NvSampleACPProcess
    };
    NvMediaIPPPluginFuncs nvbepluginFuncs = {
        .createFunc = &NvMediaBEPCreate,
        .destroyFunc = &NvMediaBEPDestroy,
        .processExFunc = &NvMediaBEPProcessEx
    };

    for (i=0; i<ctx->ippNum; i++) {
        // Create Control Algorithm component (needs valid ISP)
        if (ctx->ispEnabled[i] && ctx->controlAlgorithmEnabled[i]) {
            memset(&controlAlgorithmConfig, 0, sizeof(NvMediaIPPControlAlgorithmComponentConfig));

            controlAlgorithmConfig.width = ctx->inputWidth;
            controlAlgorithmConfig.height = ctx->inputHeight;
            controlAlgorithmConfig.pixelOrder = ctx->pixelOrder;
            controlAlgorithmConfig.bitsPerPixel = ctx->bitsPerPixel;
            controlAlgorithmConfig.iscSensorDevice = ctx->extImgDevice->iscSensor[i];
            if(ctx->pluginFlag == NVMEDIA_SIMPLEACPLUGIN) {
                controlAlgorithmConfig.pluginFuncs = &samplepluginFuncs;
            }
            else if(ctx->pluginFlag == NVMEDIA_NVACPLUGIN){
                controlAlgorithmConfig.pluginFuncs = &nvpluginFuncs;
            }
            else if(ctx->pluginFlag == NVMEDIA_NVBEPLUGIN){
                controlAlgorithmConfig.pluginFuncs = &nvbepluginFuncs;
            }

            bufferPools[0] = &bufferPool;
            bufferPools[1] = NULL;
            IPPSetControlAlgorithmBufferPoolConfig(ctx, &bufferPool);

            ctx->ippControlAlgorithmComponents[i] =
                    NvMediaIPPComponentCreateNew(ctx->ipp[i],                       //ippPipeline
                                              NVMEDIA_IPP_COMPONENT_ALG, //componentType
                                              bufferPools,                          //bufferPools
                                              &controlAlgorithmConfig);                 //componentConfig
            if (!ctx->ippControlAlgorithmComponents[i]) {
                LOG_ERR("%s: Failed to create Control Algorithm \
                         component for pipeline %d\n", __func__, i);
                goto failed;
            }
            LOG_DBG("%s: NvMediaIPPComponentCreateNew Control Algorithm\n", __func__);
        }
    }

    return NVMEDIA_STATUS_OK;
failed:
    return NVMEDIA_STATUS_ERROR;
}

// Create Output component
static NvMediaStatus IPPCreateOutputComponent(IPPCtx *ctx)
{
    NvU32 i;
    for (i=0; i<ctx->ippNum; i++) {
        // Create output component
        if (ctx->outputEnabled[i]) {
            ctx->ippComponents[i][ctx->componentNum[i]] =
                    NvMediaIPPComponentCreateNew(ctx->ipp[i],                     //ippPipeline
                                              NVMEDIA_IPP_COMPONENT_OUTPUT,    //componentType
                                              NULL,                            //bufferPools
                                              NULL);                           //componentConfig
            if (!ctx->ippComponents[i][ctx->componentNum[i]]) {
                LOG_ERR("%s: Failed to create output component \
                         for pipeline %d\n", __func__, i);
                goto failed;
            }

            LOG_DBG("%s: NvMediaIPPComponentCreateNew Output\n", __func__);
            ctx->outputComponent[i] = ctx->ippComponents[i][ctx->componentNum[i]];
            ctx->componentNum[i]++;
        }
    }

    return NVMEDIA_STATUS_OK;
failed:
    return NVMEDIA_STATUS_ERROR;
}

// Add components to Pipeline
static NvMediaStatus IPPAddComponentsToPipeline(IPPCtx *ctx)
{
    NvU32 i, j;
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;

    // Building IPPs
    for (i=0; i<ctx->ippNum; i++) {
        for (j=1; j<ctx->componentNum[i]; j++) {
            status = NvMediaIPPComponentAttach(ctx->ipp[i],                 //ippPipeline
                                      ctx->ippComponents[i][j-1],           //srcComponent,
                                      ctx->ippComponents[i][j],             //dstComponent,
                                      NVMEDIA_IPP_PORT_IMAGE_1);            //portType

            if (status != NVMEDIA_STATUS_OK) {
               LOG_ERR("%s: NvMediaIPPComponentAttach failed \
                        for IPP %d, component %d -> %d", __func__, i, j-1, j);
               goto failed;
            }
        }
#if 1
        if(ctx->ippIspComponents[i] && ctx->ippControlAlgorithmComponents[i]) {
            status = NvMediaIPPComponentAttach(ctx->ipp[i],                     //ippPipeline
                                      ctx->ippIspComponents[i],                 //srcComponent,
                                      ctx->ippControlAlgorithmComponents[i],    //dstComponent,
                                      NVMEDIA_IPP_PORT_STATS_1);                //portType

            if (status != NVMEDIA_STATUS_OK) {
               LOG_ERR("%s: NvMediaIPPComponentAttach failed for \
                        IPP %d, component ISP -> Control Algorithm", __func__, i);
               goto failed;
            }
        }
        if(ctx->ippControlAlgorithmComponents[i] && ctx->ippIscComponents[i]) {
            status = NvMediaIPPComponentAttach(ctx->ipp[i],                          //ippPipeline
                                      ctx->ippControlAlgorithmComponents[i],         //srcComponent,
                                      ctx->ippIscComponents[i],                      //dstComponent,
                                      NVMEDIA_IPP_PORT_SENSOR_CONTROL_1);            //portType

            if (status != NVMEDIA_STATUS_OK) {
               LOG_ERR("%s: NvMediaIPPComponentAttach failed for \
                       IPP %d, component Control Algorithm -> ISC", __func__, i);
               goto failed;
            }
        }
#endif
    }

    return NVMEDIA_STATUS_OK;
failed:
    return NVMEDIA_STATUS_ERROR;
}

// Create Raw Pipeline
NvMediaStatus IPPCreateRawPipeline(IPPCtx*ctx) {
    NvMediaStatus status = NVMEDIA_STATUS_ERROR;
    NvU32 i;

    if(!ctx) {
        LOG_ERR("%s: Bad parameters \n", __func__);
        goto failed;
    }

    // Create IPPs
    for (i=0; i<ctx->ippNum; i++) {
        ctx->ipp[i] = NvMediaIPPPipelineCreate(ctx->ippManager);
        if(!ctx->ipp[i]) {
            LOG_ERR("%s: Failed to create ipp %d\n", __func__, i);
            goto failed;
        }
    }

    // Build IPP components for each IPP pipeline
    memset(ctx->ippComponents, 0, sizeof(ctx->ippComponents));
    memset(ctx->componentNum, 0, sizeof(ctx->componentNum));

#if 1
    // Create Sensor Control Component
    status = IPPCreateSensorControlComponent(ctx);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to create Sensor Control Component \n", __func__);
        goto failed;
    }
#endif
    // Create Capture component
    if(ctx->useVirtualChannels) {
        status = IPPCreateCaptureComponentEx(ctx);
    } else {
        status = IPPCreateCaptureComponent(ctx);
    }

    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to create Capture Component \n", __func__);
        goto failed;
    }

    // Create ISP Component
    status = IPPCreateISPComponent(ctx);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to create ISP Component \n", __func__);
        goto failed;
    }
#if 1
    // Create Control Algorithm Component
    status = IPPCreateControlAlgorithmComponent(ctx);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to create Control algorithm Component \n", __func__);
        goto failed;
    }
#endif
    // Create Output component
    status = IPPCreateOutputComponent(ctx);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to create Output Component \n", __func__);
        goto failed;
    }

    status = IPPAddComponentsToPipeline(ctx);
    if(status != NVMEDIA_STATUS_OK) {
        LOG_ERR("%s: Failed to build IPP components \n", __func__);
        goto failed;
    }

    return NVMEDIA_STATUS_OK;
failed:
    return NVMEDIA_STATUS_ERROR;
}

