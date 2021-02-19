/*********************************************************************************************
 *
 * @file RootOfTrustNode.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief The RootOfTrustNode object is used by various processes within the secure virtual
 * machine in order to access the various facilities offered by the Root-of-Trust process. In
 * this current iteration, everything is hard coded and there is no actual connection made to
 * the root of trust. This allows developers to work on their individual components without
 * waiting for a functional Root-of-Trust process to be made available.
 *
 ********************************************************************************************/

#include "64BitHashes.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "IpcTransactionHelperFunctions.h"
#include "RootOfTrustNode.h"
#include "SocketClient.h"

#include <iostream>

/********************************************************************************************
 *
 * @class RootOfTrustNode
 * @function RootOfTrustNode
 * @brief Constructor
 * @param[in] c_szIpcPathForInitialization Path to the temporary Ipc channel used to initialize the root of trust
 *
 ********************************************************************************************/

RootOfTrustNode::RootOfTrustNode(
    _in const char * c_szIpcPathForInitialization
    )
{
    __DebugFunction();

    Socket * poSocket = ::ConnectToUnixDomainSocket(c_szIpcPathForInitialization);
    StructuredBuffer oInitializationData(::GetIpcTransaction(poSocket));
    
    // Get the information
    m_oDomainIdentifier = oInitializationData.GetGuid("YourDomainIdentifier");
    m_strRootOfTrustIpcPath = oInitializationData.GetString("RootOfTrustIpcPath");
    // Send a success response
    StructuredBuffer oResponse;
    oResponse.PutBoolean("Success", true);
    ::PutIpcTransaction(poSocket, oResponse);
    poSocket->Release();
    
    // Now that we have the basic information we need, let's fetch a bunch of basic information
    poSocket = ::ConnectToUnixDomainSocket(m_strRootOfTrustIpcPath.c_str());
    StructuredBuffer oTransactionData;
    oTransactionData.PutGuid("DomainIdentifier", m_oDomainIdentifier);
    oTransactionData.PutDword("Transaction", 0x00000007);
    std::vector<Byte> stlSerializedResponse = ::PutIpcTransactionAndGetResponse(poSocket, oTransactionData);
    StructuredBuffer oTransactionResponse(stlSerializedResponse);
    poSocket->Release();
    
    m_stlGlobalRootKeyCertificate = oTransactionResponse.GetBuffer("GlobalRootKeyCertificate");
    m_stlDataDomainRootKeyCertificate = oTransactionResponse.GetBuffer("DataDomainRootKeyCertificate");
    m_stlComputationalDomainRootKeyCertificate = oTransactionResponse.GetBuffer("ComputationalDomainRootKeyCertificate");
}

/********************************************************************************************
 *
 * @class RootOfTrustNode
 * @function RootOfTrustNode
 * @brief Copy constructor
 * @note
 *    All this copy constructor does is trigger an assertion failure if called. This is
 *    because this object should never get copied.
 *
 ********************************************************************************************/
 
RootOfTrustNode::RootOfTrustNode(
    _in const RootOfTrustNode & c_oRootOfTrustNode
    )
{
    __DebugFunction();
    //__DebugAssert(false);
    
    //UNREFERENCED_PARAMETER(c_oRootOfTrustNode);
    m_bProcessType=c_oRootOfTrustNode.m_bProcessType;
    m_oDomainIdentifier= c_oRootOfTrustNode.m_oDomainIdentifier;
    m_strRootOfTrustIpcPath = c_oRootOfTrustNode.m_strRootOfTrustIpcPath;
    m_stlGlobalRootKeyCertificate = c_oRootOfTrustNode.m_stlGlobalRootKeyCertificate;
    m_stlComputationalDomainRootKeyCertificate = c_oRootOfTrustNode.m_stlComputationalDomainRootKeyCertificate;
    m_stlDataDomainRootKeyCertificate = c_oRootOfTrustNode.m_stlDataDomainRootKeyCertificate;
}

/********************************************************************************************
 *
 * @class RootOfTrustNode
 * @function ~RootOfTrustNode
 * @brief Destructor
 *
 ********************************************************************************************/
 
RootOfTrustNode::~RootOfTrustNode(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class RootOfTrustNode
 * @function GetEphemeralTlsKeyPairAndCertificate
 * @brief Generate a cryptographic key pair and associated digitally signed certificate which
 * for the purposes of being used as a TLS server identity.
 * @return A StructuredBuffer containing three buffers, each representing a PEM encoded value.
 * @note
 *    The type of cryptographic key served up depends on policies that will eventually exist
 *    (but do not currently). Policies selected by customers will instruct the Root-of-Trust
 *    on the type and strength of asymmetric key to generate (i.e. RSA vs. ECC, etc...) and
 *    will also instruct the acceptable policies when it comes to signing certificates. For
 *    not, everything is hardcoded
 *
 ********************************************************************************************/
 
StructuredBuffer __thiscall RootOfTrustNode::GetEphemeralTlsKeyPairAndCertificate(void) const
{
    __DebugFunction();
    
    // Now that we have the basic information we need, let's fetch a bunch of basic information
    Socket * poSocket = ::ConnectToUnixDomainSocket(m_strRootOfTrustIpcPath.c_str());
    StructuredBuffer oTransactionData;
    oTransactionData.PutGuid("DomainIdentifier", m_oDomainIdentifier);
    oTransactionData.PutDword("Transaction", 0x00000008);
    StructuredBuffer oTransactionResponse(::PutIpcTransactionAndGetResponse(poSocket, oTransactionData));
    poSocket->Release();
    
    return oTransactionResponse;
}

/********************************************************************************************
 *
 * @class RootOfTrustNode
 * @function GetRootPublicKeyAndCertificate
 * @brief Fetches the public key and associated certificate for the Root Key
 * @return A StructuredBuffer containing two buffers, each representing a PEM encoded value.
 *
 ********************************************************************************************/
 
std::vector<Byte> __thiscall RootOfTrustNode::GetGlobalRootKeyCertificate(void) const
{
    __DebugFunction();
    
    return m_stlGlobalRootKeyCertificate;
}

/********************************************************************************************
 *
 * @class RootOfTrustNode
 * @function GetComputationalDomainRootKeyCertificate
 * @brief Fetches the public key and associated certificate for the Computational Domain
 * @return A StructuredBuffer containing two buffers, each representing a PEM encoded value.
 *
 ********************************************************************************************/
 
std::vector<Byte> __thiscall RootOfTrustNode::GetComputationalDomainRootKeyCertificate(void) const
{
    __DebugFunction();
    
    return m_stlComputationalDomainRootKeyCertificate;
}

/********************************************************************************************
 *
 * @class RootOfTrustNode
 * @function GetDataDomainRootKeyCertificate
 * @brief Fetches the public key and associated certificate for the Data Domain
 * @return A StructuredBuffer containing two buffers, each representing a PEM encoded value.
 *
 ********************************************************************************************/
 
std::vector<Byte> __thiscall RootOfTrustNode::GetDataDomainRootKeyCertificate(void) const
{
    __DebugFunction();
    
    return m_stlDataDomainRootKeyCertificate;
}

/********************************************************************************************/
 
std::string __thiscall RootOfTrustNode::GetDataDomainIpcPath(void) const
{
    __DebugFunction();
    
    Socket * poSocket = ::ConnectToUnixDomainSocket(m_strRootOfTrustIpcPath.c_str());
    StructuredBuffer oTransactionData;
    oTransactionData.PutGuid("DomainIdentifier", m_oDomainIdentifier);
    oTransactionData.PutDword("Transaction", 0x00000001);
    StructuredBuffer oTransactionResponse(::PutIpcTransactionAndGetResponse(poSocket, oTransactionData));
    poSocket->Release();
    
    return oTransactionResponse.GetString("DataDomainIpcPath");
}

/********************************************************************************************/
 
std::string __thiscall RootOfTrustNode::GetComputationalDomainIpcPath(void) const
{
    __DebugFunction();
    
    Socket * poSocket = ::ConnectToUnixDomainSocket(m_strRootOfTrustIpcPath.c_str());
    StructuredBuffer oTransactionData;
    oTransactionData.PutGuid("DomainIdentifier", m_oDomainIdentifier);
    oTransactionData.PutDword("Transaction", 0x00000003);
    StructuredBuffer oTransactionResponse(::PutIpcTransactionAndGetResponse(poSocket, oTransactionData));
    poSocket->Release();
    
    return oTransactionResponse.GetString("ComputationalDomainIpcPath");
}

/********************************************************************************************/

StructuredBuffer __thiscall RootOfTrustNode::GetDigitalContract(void) const
{
    __DebugFunction();
    
    Socket * poSocket = ::ConnectToUnixDomainSocket(m_strRootOfTrustIpcPath.c_str());
    StructuredBuffer oTransactionData;
    oTransactionData.PutGuid("DomainIdentifier", m_oDomainIdentifier);
    oTransactionData.PutDword("Transaction", 0x00000005);
    StructuredBuffer oTransactionResponse(::PutIpcTransactionAndGetResponse(poSocket, oTransactionData));
    poSocket->Release();
    
    return oTransactionResponse;
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustNode::GetDataset(void) const
{
    __DebugFunction();
    
    Socket * poSocket = ::ConnectToUnixDomainSocket(m_strRootOfTrustIpcPath.c_str());
    StructuredBuffer oTransactionData;
    oTransactionData.PutGuid("DomainIdentifier", m_oDomainIdentifier);
    oTransactionData.PutDword("Transaction", 0x00000006);
    StructuredBuffer oTransactionResponse(::PutIpcTransactionAndGetResponse(poSocket, oTransactionData));
    poSocket->Release();

    return oTransactionResponse.GetBuffer("DataSet");
}

/********************************************************************************************/

void __thiscall RootOfTrustNode::RecordAuditEvent(
    _in const std::string & c_oEncryptedOpaqueSessionBlob,
    _in Dword dwEventType,
    _in const StructuredBuffer & c_oEventData
    ) const
{
    __DebugFunction();
    try
    {
        Socket * poSocket = ::ConnectToUnixDomainSocket(m_strRootOfTrustIpcPath.c_str());
        StructuredBuffer oTransactionData;
        oTransactionData.PutString("Eosb", c_oEncryptedOpaqueSessionBlob);
        oTransactionData.PutGuid("DomainIdentifier", m_oDomainIdentifier);
        oTransactionData.PutDword("Transaction", 0x00000009);
        oTransactionData.PutDword("EventType", dwEventType);
        oTransactionData.PutStructuredBuffer("EventData", c_oEventData);
        //StructuredBuffer oTransactionResponse(::PutIpcTransactionAndGetResponse(poSocket, oTransactionData));
        ::PutIpcTransactionAndGetResponse(poSocket, oTransactionData);
        poSocket->Release();
    }
    catch (BaseException oException)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
    catch (...)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
}
