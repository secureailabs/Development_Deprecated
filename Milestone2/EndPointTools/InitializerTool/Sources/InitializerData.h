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

#include <map>
#include <string>
#include <vector>

/********************************************************************************************/

class InitializerData : public Object
{
    public:
    
        InitializerData(void);
        virtual ~InitializerData(void);
       
        bool __thiscall Login(
            _in const std::string strOrganization,
            _in const std::string strUsername,
            _in const std::string strPassword
            );
        
        std::vector<std::string> __thiscall GetListOfDigitalContracts(void) const;
        std::string __thiscall GetEffectiveDigitalContractName(void) const throw();
        void __thiscall SetEffectiveDigitalContract(
            _in const std::string & c_strEffectiveDigitalContract
            );
        
        std::vector<std::string> __thiscall GetUninitializedNodeAddresses(void) const;
        std::vector<std::string> __thiscall GetClusterNodeAddresses(void) const;
        bool __thiscall AddNodeToCluster(
            _in const std::string & c_strNodeAddress
            );
        void __thiscall RemoveNodeFromCluster(
            _in const std::string & c_strNodeAddress
            );
        
        std::string __thiscall GetDatasetFilename(void) const throw();
        bool __thiscall SetDatasetFilename(
            _in const std::string & c_strDatasetFilename
            );
        
        bool __thiscall InitializeNode(
            _in const std::string & c_strNodeAddress
            ) const;
            
    private:
    
        // Private methods
        bool __thiscall GetImposterEncryptedOpaqueSessionBlob(void);
        std::string __thiscall SendSaasRequest(
            _in const std::string c_strVerb,
            _in const std::string c_strResource,
            _in const std::string & c_strBody
            );

        // Private data members
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