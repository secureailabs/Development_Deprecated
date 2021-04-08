/*********************************************************************************************
 *
 * @file RootOfTrustCore.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "Socket.h"
#include "SmartMemoryAllocator.h"

#include <mutex>
#include <queue>
#include <string>
#include <vector>

/********************************************************************************************/

class RootOfTrustCore : public Object
{
    public:
    
        RootOfTrustCore(
            _in const std::vector<Byte> & c_stlSerializedInitializationParameters
            );
        RootOfTrustCore(
            _in const RootOfTrustCore & c_oRootOfTrust
            );
        virtual ~RootOfTrustCore(void);
        
        Guid __thiscall GetDataDomainIdentifier(void) const throw();
        std::string __thiscall GetRootOfTrustIpcPath(void) const throw();
        
        void __thiscall AuditEventDispatcher(void);
        
        void __thiscall RunIpcListener(void);
        void __thiscall HandleIncomingTransaction(
            _in Socket * poSocket
            );
        void __thiscall WaitForTermination(void) throw();
        
    private:
    
        // Private methods dedicated to handling incoming transactions
        
        std::vector<Byte> __thiscall TransactGetDataSet(
            _in const Guid & c_oOriginatingDomainIdentifier
            );
        std::vector<Byte> __thiscall TransactRecordAuditEvent(
            _in const Guid & c_oOriginatingDomainIdentifier,
            _in const StructuredBuffer & c_oTransactionParameters
            );
        bool __thiscall InitializeVirtualMachine(void);
        bool __thiscall InitializeDataset(void);
        bool __thiscall RegisterDataOwnerEosb(void);
        bool __thiscall RegisterResearcherEosb(void);
        
        // Private data members
        
        bool m_fIsInitialized;
        bool m_fIsRunning;

        std::string m_strDataOwnerAccessToken;
        std::string m_strResearcherEosb;
        std::string m_strVirtualMachineEosb;

        std::string m_strNameOfVirtualMachine;
        std::string m_strIpAddressOfVirtualMachine;
        std::string m_strVirtualMachineIdentifier;
        std::string m_strClusterIdentifier;
        std::string m_strDigitalContractIdentifier;
        std::string m_strDatasetIdentifier;
        std::string m_strRootOfTrustDomainIdentifier;
        std::string m_strComputationalDomainIdentifier;
        std::string m_strDataConnectorDomainIdentifier;
        std::string m_strSailWebApiPortalIpAddress;
        std::string m_strDataOwnerOrganizationIdentifier;
        std::string m_strDataOwnerUserIdentifier;
        std::vector<Byte> m_stlDataset;
    
        std::string m_strRootOfTrustIpcPath;
        std::string m_strComputationalDomainIpcPath;
        std::string m_strDataDomainIpcPath;
        
        std::mutex m_stlAuditEventsMutex;
        std::string m_strDataOrganizationAuditEventParentBranchNodeIdentifier;
        std::string m_strResearcherOrganizationAuditEventParentBranchNodeIdentifier;
        std::queue<std::string> m_stlDataOrganizationAuditEventQueue;
        std::queue<std::string> m_stlResearchOrganizationAuditEventQueue;
        std::queue<std::string> m_stlIndependentAuditorOrganizationAuditEventQueue;
        std::queue<std::string> m_stlSailOrganizationAuditEventQueue;
};