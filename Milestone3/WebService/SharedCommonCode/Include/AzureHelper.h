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

std::string CreateAzureParamterJson(
    _in const std::string & c_strTemplateUrl,
    _in const StructuredBuffer & c_oStructuredBuffer
    );

std::string CreateAzureResourceId(
    _in const std::string & c_strSubscriptionIdentifier,
    _in const std::string & c_strResourceGroup,
    _in const std::string & c_strResourceProviderNamespace,
    _in const std::string & c_strResourceType,
    _in const std::string & c_strResourceName
    );

bool DoesAzureResourceExist(
    _in const std::string & c_strMicrosoftAzureAccessToken,
    _in const std::string & c_strResourceId
    );

std::string __stdcall LoginToMicrosoftAzureApiPortal(
    _in const std::string & c_szApplicationIdentifier,
    _in const std::string & c_szSecret,
    _in const std::string & c_szTenantIdentifier
    ) throw();

StructuredBuffer CreateAzureDeployment(
    _in const std::string & c_strAzureAccessToken,
    _in const std::string & c_strdeploymentParameters,
    _in const std::string & c_strSubscriptionIdentifier,
    _in const std::string & c_strResourceGroup,
    _in const std::string & c_strLocation
    ) throw();

StructuredBuffer CopyVirtualMachineImage(
    _in const std::string c_strMicrosoftAzureAccessToken,
    _in const std::string c_strSubscriptionId,
    _in const std::string c_strResourceGroupName,
    _in const std::string c_strLocation,
    _in const std::string c_strImageName
    ) throw();

StructuredBuffer DeployVirtualMachineAndWait(
    _in const std::string & c_strApplicationIdentifier,
    _in const std::string & c_strSecret,
    _in const std::string & c_strTenantIdentifier,
    _in const std::string & c_strSubscriptionIdentifier,
    _in const std::string & c_strResourceGroup,
    _in const std::string & c_strVirtualMachineIdentifier,
    _in const std::string & c_strconfidentialVirtualMachineSpecification,
    _in const std::string & c_strLocation
    ) throw();

bool DeleteAzureResources(
    _in const std::string & c_strApplicationIdentifier,
    _in const std::string & c_strTenantIdentifier,
    _in const std::string & c_strSecret,
    _in const std::vector<std::string> & c_stlResourceId
    );

std::vector<std::string> AzureResourcesAssociatedWithVirtualMachine(
    const std::string & c_strSubscriptionID,
    const std::string & c_strResourceGroup,
    const std::string & c_strVirtualMachineName
);
