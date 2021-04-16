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
        Guid __thiscall GetComputationalDomainIdentifier(void) const throw();
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
        void __thiscall RecordInternalAuditEvent(
            _in const char * c_szEventName,
            _in Word wTargetChannelsBitMask,
            _in Dword dwEventType,
            _in const StructuredBuffer & c_oEventData
            );
};