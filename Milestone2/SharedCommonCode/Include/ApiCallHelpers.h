/*********************************************************************************************
 *
 * @file ApiCallHelpers.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Guid.h"
#include "StructuredBuffer.h"

#include <string>

extern bool __stdcall SetIpAddressOfSailWebApiPortalGateway(
    _in const std::string & c_strIpAddressOfWebPortalGateway,
    _in Word wPortAddressOfWebPortalGateway
    ) throw();
    
extern std::string __stdcall LoginToSailWebApiPortal(
    _in const std::string & c_strUsername,
    _in const std::string & c_strUserPassword
    );
    
extern StructuredBuffer __stdcall GetBasicUserInformation(
    _in const std::string & c_strEosb
    );
    
extern bool __stdcall TransmitAuditEventsToSailWebApiPortal(
    _in const std::string & c_strEosb,
    _in const std::string & c_strParentBranchNodeIdentifier,
    _in const StructuredBuffer & c_oAuditEvents
    );
    
extern std::string __stdcall RegisterVirtualMachineWithSailWebApiPortal(
    _in const std::string & c_strEosb,
    _in const std::string & c_strVirtualMachineIdentifier,
    _in const std::string & c_strDigitalContractIdentifier,
    _in const std::string & c_strIpAddress
    );
    
extern std::string __stdcall RegisterVirtualMachineDataOwner(
    _in const std::string & c_strEosb,
    _in const std::string & c_strVirtualMachineIdentifier
    );
    
extern std::string RegisterVirtualMachineResearcher(
    _in const std::string & c_strEosb,
    _in const std::string & c_strVirtualMachineIdentifier
    );