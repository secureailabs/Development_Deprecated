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
        
        Guid __thiscall GetRootOfTrustDomainIdentifier(void) const throw();
        Guid __thiscall GetComputationalDomainIdentifier(void) const throw();
        Guid __thiscall GetDataDomainIdentifier(void) const throw();
        
        std::string __thiscall GetRootOfTrustIpcPath(void) const throw();
        
        void __thiscall RunIpcListener(void);
        void __thiscall HandleIncomingTransaction(
            _in Socket * poSocket
            );
        void __thiscall WaitForTermination(void) throw();
        
    private:
    
        // Private methods dedicated to handling incoming transactions
        
        std::vector<Byte> __thiscall TransactGetDataDomainIpcPath(
            _in const Guid & c_oOriginatingDomainIdentifier
            );
        std::vector<Byte> __thiscall TransactGetDataDomainRootKeyCertificate(
            _in const Guid & c_oOriginatingDomainIdentifier
            );
        std::vector<Byte> __thiscall TransactGetComputationalDomainIpcPath(
            _in const Guid & c_oOriginatingDomainIdentifier
            );
        std::vector<Byte> __thiscall TransactGetComputationalDomainRootKeyCertificate(
            _in const Guid & c_oOriginatingDomainIdentifier
            );
        std::vector<Byte> __thiscall TransactGetDigitalContract(
            _in const Guid & c_oOriginatingDomainIdentifier
            );
        std::vector<Byte> __thiscall TransactGetDataSet(
            _in const Guid & c_oOriginatingDomainIdentifier
            );
        std::vector<Byte> __thiscall TransactGetAllDomainCertificates(
            _in const Guid & c_oOriginatingDomainIdentifier
            );
        std::vector<Byte> __thiscall TransactGenerateEphemeralTlsKeyPair(
            _in const Guid & c_oOriginatingDomainIdentifier
            );
    
        // Private data members
        
        bool m_fIsInitialized;
        bool m_fIsRunning;
        
        std::vector<Byte> m_stlSerializedDigitalContract;
        std::vector<Byte> m_stlDataOwnerImpostorEncryptedOpaqueSessionBlob;
        std::vector<Byte> m_stlResearcherImpostorEncryptedOpaqueSessionBlob;
        std::vector<Byte> m_stlGlobalRootKeyCertificate;
        std::vector<Byte> m_stlComputationalDomainRootKeyCertificate;
        std::vector<Byte> m_stlDataDomainRootKeyCertificate;
        
        Guid m_oClusterInstanceIdentifier;
        
        Guid m_oRootOfTrustDomainIdentifier;
        Guid m_oComputationalDomainIdentifier;
        Guid m_oDataDomainIdentifier;
        Guid m_oInitializerDomainIdentifier;
        
        std::string m_strRootOfTrustIpcPath;
        std::string m_strComputationalDomainIpcPath;
        std::string m_strDataDomainIpcPath;
        
        std::vector<Byte> m_stlDataSet;
};