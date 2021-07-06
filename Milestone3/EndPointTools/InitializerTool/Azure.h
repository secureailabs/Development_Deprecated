/*********************************************************************************************
 *
 * @file Azure.h
 * @author Prawal Gangwar
 * @date 03 Feb 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the Azure class to interact with Azure Web APIs
 *
 ********************************************************************************************/
#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "Guid.h"
#include "StructuredBuffer.h"
#include "JsonValue.h"

#include <string>

/********************************************************************************************/

std::string __stdcall GeneratePassword(
    _in const unsigned int c_unPasswordLength
    );

class Azure : public Object
{
    public:

        Azure() = delete;
        Azure(
            _in const Azure & c_oAzure
        ) = delete;
        Azure(
            _in const std::string & c_strAppId,
            _in const std::string & c_strSecret,
            _in const std::string & c_strSubscriptionID,
            _in const std::string & c_strTenant,
            _in const std::string & c_strNetworkSecurityGroup,
            _in const std::string & c_strLocation
        );
        ~Azure(void);

        bool __thiscall Authenticate(void);
        std::string __thiscall ProvisionVirtualMachine(
            _in const std::string c_strBaseImageName,
            _in const std::string c_strVirtualMachineSize,
            _in const std::string c_strDnsLabel,
            _in const std::string c_strPassword
            );
        std::string __thiscall GetVmProvisioningState(
            _in std::string & c_strVirtualMachineName
            );
        std::string __thiscall GetVmIp(
            _in std::string & c_strVmId
            );
        std::string __thiscall CompleteTemplate(
            _in std::string c_strFileName
            );
        bool __thiscall DeleteVirtualMachine(
            _in const std::string & c_strVmName
            );
        void __thiscall SetResourceGroup(
            _in const std::string c_strResourceGroupName
            );
        bool __thiscall CreateResourceGroup(
            _in const std::string c_strResourceGroupName
            );
        std::string __thiscall CreateVirtualNetwork(
            _in std::string c_strVirtualNetworkName
            );
        void __thiscall SetVirtualNetwork(
            _in const std::string c_strVirtualNetworkName
            );
        void __thiscall WaitToRun(
            _in const std::string & strVmName
            );

        // TODO:
        bool __thiscall AssignImageAccess(
            _in const std::string c_strPrincipalId
            );
        bool __thiscall RevokeImageAccess(
            _in const std::string c_strPrincipalId
            );

    private:

        std::string m_strAuthToken;
        const std::string m_strAppId;
        const std::string m_strSecret;
        const std::string m_strTenant;
        const std::string m_strSubscriptionId;
        const std::string m_strNetworkSecurityGroup;
        std::string m_strResourceGroup;
        std::string m_strVirtualNetwork;
        const std::string m_strLocation;

        std::string __thiscall MakeRestCall(
            _in const std::string c_strVerb,
            _in const std::string c_strResource,
            _in const std::string c_strHost,
            _in const std::string & c_strBody,
            _in const std::string c_strApiVersionDate
        );
};
