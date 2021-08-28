/*********************************************************************************************
 *
 * @file AzureHelper.h
 * @author Prawal Gangwar
 * @date 28 July 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "StructuredBuffer.h"

#include <string>

std::string DeployVirtualMachineAndWait(
    _in const std::string & c_strApplicationIdentifier,
    _in const std::string & c_strSecret,
    _in const std::string & c_strTenantIdentifier,
    _in const std::string & c_strSubscriptionIdentifier,
    _in const std::string & c_strResourceGroup,
    _in const std::string & c_strVirtualMachineIdentifier,
    _in const std::string & c_strconfidentialVirtualMachineSpecification,
    _in const std::string & c_strLocation
);

std::string CreateAzureParamterJson(
    _in const std::string & c_strTemplateUrl,
    _in const StructuredBuffer & c_oStructuredBuffer
);

bool DoesAzureResourceExist(
    _in const std::string & c_strApplicationIdentifier,
    _in const std::string & c_strSecret,
    _in const std::string & c_strTenantIdentifier,
    _in const std::string & c_strResourceId
);

std::string CreateAzureResourceId(
    _in const std::string & c_strSubscriptionIdentifier,
    _in const std::string & c_strResourceGroup,
    _in const std::string & c_strResourceProviderNamespace,
    _in const std::string & c_strResourceType,
    _in const std::string & c_strResourceName
);

std::string CreateAzureDeployment(
    _in const std::string & c_strVirtualNetworkName,
    _in const std::string & c_strApplicationIdentifier,
    _in const std::string & c_strSecret,
    _in const std::string & c_strTenantIdentifier,
    _in const std::string & c_strSubscriptionIdentifier,
    _in const std::string & c_strResourceGroup,
    _in const std::string & c_strLocation
);

bool DeleteAzureResources(
    _in const std::string & c_strApplicationIdentifier,
    _in const std::string & c_strTenantIdentifier,
    _in const std::string & c_strSecret,
    _in const std::vector<std::string> & c_stlResourceId
);

bool DoesAzureResourceExist(
    _in const std::string & c_strMicrosoftAzureAccessToken,
    _in const std::string & c_strResourceId
);

std::string CopyVirtualMachineImage(
    _in const std::string c_strSubscriptionId,
    _in const std::string c_strApplicationIdentifier,
    _in const std::string c_strSecret,
    _in const std::string c_strTenantIdentifier,
    _in const std::string c_strResourceGroupName,
    _in const std::string c_strLocation,
    _in const std::string c_strImageName
);
