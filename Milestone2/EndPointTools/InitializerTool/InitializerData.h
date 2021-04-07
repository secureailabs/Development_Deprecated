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

#ifndef SERVER_PORT
    #define SERVER_PORT 6200
#endif

/********************************************************************************************/

const std::string gc_strSubscriptionId = "20c11edd-abb4-4bc0-a6d5-c44d6d2524be";
const std::string gc_strLocation = "eastus2";
const std::string gc_strResourceGroup = "ComputationVmTemplate0.6_group";
const std::string gc_strImageName = "ComputationVmTemplate0.6Image";
const std::string gc_strVirtualNetwork = "ComputationVmTemplate0.6_group-vnet";
const std::string gc_strNetworkSecurityGroup = "ComputationVmTemplate0.6-nsg";
const std::string gc_strVirtualMachineSize = "Standard_B1ms";

/********************************************************************************************/

class InitializerData : public Object
{
    public:

        virtual ~InitializerData(void);
        InitializerData(
            _in const InitializerData & c_oInitializerData
            ) = delete;
        // The static function for the class that can get the reference to the
        // InitializerData singleton object
        static InitializerData & Get(void);

        bool __thiscall Login(
            _in const std::string c_strWebServiceIp,
            _in const std::string strUsername,
            _in const std::string strPassword
            );

        std::vector<std::string> __thiscall GetListOfDigitalContracts(void) const;
        std::string __thiscall GetEffectiveDigitalContractName(
            _in const unsigned int unDigitalContractIndex
            ) const throw();
        void __thiscall AddEffectiveDigitalContract(
            _in const std::string & c_strEffectiveDigitalContract
            );

        std::string __thiscall GetDatasetFilename(
            _in const unsigned int unFilenameIndex
        ) const throw();
        bool __thiscall AddDatasetFilename(
            _in const std::string & c_strDatasetFilename
            );

        bool __thiscall InitializeNode(
            _in const std::string & c_strNodeAddress,
            _in unsigned int unDatasetIndex
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
        unsigned int __thiscall GetNumberOfDatasets(void) const;
        unsigned int __thiscall CreateAndInitializeVirtualMachine(
                _in unsigned int unDatasetIndex
            );

    private:

        // Private methods
        InitializerData(void);
        static InitializerData m_oInitializerData;

        bool __thiscall GetImposterEncryptedOpaqueSessionBlob(void);
        std::string __thiscall SendSaasRequest(
            _in const std::string & c_strWebservice,
            _in const std::string c_strVerb,
            _in const std::string c_strResource,
            _in const std::string & c_strBody
            ) const;

        // Private data members
        Azure * m_poAzure = nullptr;
        pthread_mutex_t m_sMutex;
        unsigned int m_unNumberOfVirtualMachines;
        std::string m_stlEncryptedOpaqueSessionBlob;
        std::string m_stlImposterEncryptedOpaqueSessionBlob;
        Guid m_oClusterIdentifier;
        Guid m_oDataDomainIdentifier;
        Guid m_oComputationalDomainIdentifier;
        Guid m_oRootOfTrustDomainIdentifier;
        std::vector<std::string> m_stlDatasetFilename;
        std::vector<std::string> m_stlEffectiveDigitalContractName;
        std::vector<Byte> m_stlEffectiveDigitalContract;
        std::map<Qword, std::string> m_stlClusterNodes;
        std::vector<Byte> m_stlDataset;
};