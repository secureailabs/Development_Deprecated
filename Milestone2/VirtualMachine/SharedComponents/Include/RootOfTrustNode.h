/*********************************************************************************************
 *
 * @file RootOfTrustNode.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "StructuredBuffer.h"
#include <string>
#include <vector>

/********************************************************************************************/

class RootOfTrustNode : public Object
{
    public:
    
        RootOfTrustNode(
            _in const char * c_szIpcPathForInitialization
            );
        RootOfTrustNode(
            _in const RootOfTrustNode & c_oRootOfTrust
            );
        virtual ~RootOfTrustNode(void);
        
        StructuredBuffer __thiscall GetEphemeralTlsKeyPairAndCertificate(void) const;
        
        std::vector<Byte> __thiscall GetGlobalRootKeyCertificate(void) const;
        std::vector<Byte> __thiscall GetComputationalDomainRootKeyCertificate(void) const;
        std::vector<Byte> __thiscall GetDataDomainRootKeyCertificate(void) const;
        
        std::string __thiscall GetDataDomainIpcPath(void) const;
        std::string __thiscall GetComputationalDomainIpcPath(void) const;
        
        StructuredBuffer __thiscall GetDigitalContract(void) const;
        std::vector<Byte> __thiscall GetDataset(void) const;
        
        void __thiscall RecordAuditEvent(
            _in const std::string & c_oEncryptedOpaqueSessionBlob,
            _in Dword dwEventType,
            _in const StructuredBuffer & c_oEventData
            ) const;
            
    private:
    
        Byte m_bProcessType;
        Guid m_oDomainIdentifier;
        
        std::string m_strRootOfTrustIpcPath;
        
        std::vector<Byte> m_stlGlobalRootKeyCertificate;
        std::vector<Byte> m_stlComputationalDomainRootKeyCertificate;
        std::vector<Byte> m_stlDataDomainRootKeyCertificate;
};
