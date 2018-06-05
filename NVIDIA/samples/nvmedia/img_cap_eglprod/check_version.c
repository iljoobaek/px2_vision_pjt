/* Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "check_version.h"
#include "log_utils.h"

NvMediaStatus
CheckModulesVersion(
                void)
{
    NvMediaVersion nvm_version;
    ExtImgDevVersion imgdev_version;
    NvMediaStatus status = NVMEDIA_STATUS_OK;

    memset(&nvm_version, 0, sizeof(NvMediaVersion));
    memset(&imgdev_version, 0, sizeof(ExtImgDevVersion));

    EXTIMGDEV_SET_VERSION(imgdev_version, EXTIMGDEV_VERSION_MAJOR,
                                          EXTIMGDEV_VERSION_MINOR);
    status = ExtImgDevCheckVersion(&imgdev_version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    memset(&nvm_version, 0, sizeof(NvMediaVersion));
    status = NvMediaCoreGetVersion(&nvm_version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if((nvm_version.major != NVMEDIA_CORE_VERSION_MAJOR) ||
       (nvm_version.minor != NVMEDIA_CORE_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible core version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_CORE_VERSION_MAJOR, NVMEDIA_CORE_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            nvm_version.major, nvm_version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&nvm_version, 0, sizeof(NvMediaVersion));
    status = NvMediaImageGetVersion(&nvm_version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if((nvm_version.major != NVMEDIA_IMAGE_VERSION_MAJOR) ||
       (nvm_version.minor != NVMEDIA_IMAGE_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible image version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_IMAGE_VERSION_MAJOR, NVMEDIA_IMAGE_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            nvm_version.major, nvm_version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&nvm_version, 0, sizeof(NvMediaVersion));
    status = NvMediaISCGetVersion(&nvm_version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if (nvm_version.major != NVMEDIA_ISC_VERSION_MAJOR ||
        nvm_version.minor != NVMEDIA_ISC_VERSION_MINOR) {
        LOG_ERR("%s: Incompatible ISC version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
                NVMEDIA_ISC_VERSION_MAJOR, NVMEDIA_ISC_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
                nvm_version.major, nvm_version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&nvm_version, 0, sizeof(NvMediaVersion));
    status = NvMediaICPGetVersion(&nvm_version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if (nvm_version.major != NVMEDIA_ICP_VERSION_MAJOR ||
        nvm_version.minor != NVMEDIA_ICP_VERSION_MINOR) {
        LOG_ERR("%s: Incompatible ICP version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
                NVMEDIA_ICP_VERSION_MAJOR, NVMEDIA_ICP_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
                nvm_version.major, nvm_version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&nvm_version, 0, sizeof(NvMediaVersion));
    status = NvMedia2DGetVersion(&nvm_version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if ((nvm_version.major != NVMEDIA_2D_VERSION_MAJOR) ||
        (nvm_version.minor != NVMEDIA_2D_VERSION_MINOR)) {
        LOG_ERR("%s: Incompatible 2D version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
            NVMEDIA_2D_VERSION_MAJOR, NVMEDIA_2D_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
            nvm_version.major, nvm_version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    memset(&nvm_version, 0, sizeof(NvMediaVersion));
    status = NvMediaIDPGetVersion(&nvm_version);
    if (status != NVMEDIA_STATUS_OK)
        return status;

    if (nvm_version.major != NVMEDIA_IDP_VERSION_MAJOR ||
        nvm_version.minor != NVMEDIA_IDP_VERSION_MINOR) {
        LOG_ERR("%s: Incompatible IDP version found \n", __func__);
        LOG_ERR("%s: Client version: %d.%d\n", __func__,
                NVMEDIA_IDP_VERSION_MAJOR, NVMEDIA_IDP_VERSION_MINOR);
        LOG_ERR("%s: Core version: %d.%d\n", __func__,
                nvm_version.major, nvm_version.minor);
        return NVMEDIA_STATUS_INCOMPATIBLE_VERSION;
    }

    return status;
}
