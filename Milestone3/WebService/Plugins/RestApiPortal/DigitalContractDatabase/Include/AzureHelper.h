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
    _in const std::string & c_szApplicationIdentifier,
    _in const std::string & c_szSecret,
    _in const std::string & c_szTenantIdentifier,
    _in const std::string & c_szSubscriptionIdentifier,
    _in const std::string & c_szResourceGroup,
    _in const std::string & c_szVirtualMachineIdentifier,
    _in const std::string & c_szconfidentialVirtualMachineSpecification,
    _in const std::string & c_szLocation
);

std::string CreateAzureParamterJson(
    _in const std::string & c_strTemplateUrl,
    _in const StructuredBuffer & c_oStructuredBuffer
);
