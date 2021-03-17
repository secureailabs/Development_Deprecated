/*********************************************************************************************
 *
 * @file InstallerData.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Guid.h"
#include "Object.h"
#include "StructuredBuffer.h"
#include "Azure.h"

#include <map>
#include <string>
#include <vector>

#ifndef SERVER_IP_ADDRESS
    #define SERVER_PORT 6200
    #define SERVER_IP_ADDRESS "127.0.0.1"
#endif

/********************************************************************************************/

const std::string gc_strSubscriptionId = "20c11edd-abb4-4bc0-a6d5-c44d6d2524be";
const std::string gc_strLocation = "eastus2";
const std::string gc_strResourceGroup = "ComputationVmTemplate0.3_group";
const std::string gc_strImageName = "ComputationVmTemplate0.3Image";
const std::string gc_strVirtualNetwork = "ComputationVmTemplate0.3_group-vnet";
const std::string gc_strNetworkSecurityGroup = "ComputationVmTemplate0.3-nsg";
const std::string gc_strVirtualMachineSize = "Standard_B1ls";

/********************************************************************************************/

class InitializerData : public Object
{
    public:
    
        InitializerData(void);
        virtual ~InitializerData(void);
       
        bool __thiscall Login(
            _in const std::string strUsername,
            _in const std::string strPassword
            );

        std::vector<std::string> __thiscall GetListOfDigitalContracts(void) const;
        std::string __thiscall GetEffectiveDigitalContractName(void) const throw();
        void __thiscall SetEffectiveDigitalContract(
            _in const std::string & c_strEffectiveDigitalContract
            );

        std::string __thiscall GetDatasetFilename(void) const throw();
        bool __thiscall SetDatasetFilename(
            _in const std::string & c_strDatasetFilename
            );

        bool __thiscall InitializeNode(
            _in const std::string & c_strNodeAddress
            ) const;
        bool __thiscall AzureLogin(
            _in const std::string & c_strAppId,
            _in const std::string & c_strSecret,
            _in const std::string & c_strSubscriptionID,
            _in const std::string & c_strNetworkSecurityGroup,
            _in const std::string & c_strLocation,
            _in const std::string & c_strTenant
            );
        void __thiscall SetNumberOfVirtualMachines(
            _in const unsigned int c_unNumberOfVirtualMachines
            );
        unsigned int __thiscall GetNumberOfVirtualMachines(void) const;
        unsigned int __thiscall CreateVirtualMachines(void);

    private:

        // Private methods
        bool __thiscall GetImposterEncryptedOpaqueSessionBlob(void);
        std::string __thiscall SendSaasRequest(
            _in const std::string c_strVerb,
            _in const std::string c_strResource,
            _in const std::string & c_strBody
            );

        // Private data members
        Azure * m_poAzure = nullptr;
        unsigned int m_unNumberOfVirtualMachines;
        std::string m_stlEncryptedOpaqueSessionBlob;
        std::string m_stlImposterEncryptedOpaqueSessionBlob;
        Guid m_oClusterIdentifier;
        Guid m_oDataDomainIdentifier;
        Guid m_oComputationalDomainIdentifier;
        Guid m_oRootOfTrustDomainIdentifier;
        std::string m_strDatasetFilename;
        std::string m_strEffectiveDigitalContractName;
        std::vector<Byte> m_stlEffectiveDigitalContract;
        std::map<Qword, std::string> m_stlClusterNodes;
        std::vector<Byte> m_stlDataset;
};